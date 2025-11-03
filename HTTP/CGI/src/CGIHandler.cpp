/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 20:13:05 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/03 17:35:56 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/CGIHandler.hpp"

#include <string>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <cctype>

static size_t hexToSize(const std::string &hex)
{
    size_t value = 0;
    for (size_t i = 0; i < hex.size(); ++i)
    {
        char c = hex[i];
        int digit = 0;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') digit = 10 + (c - 'A');
        else throw std::runtime_error("Invalid hex digit in chunk size");
        value = (value << 4) | digit;
    }
    return value;
}

// Decode an entire chunked body already read from the socket
std::string decodeChunkedBody(const std::string &raw)
{
    std::string decoded;
    size_t pos = 0;

    while (true)
    {
        // Find CRLF after the chunk-size line
        size_t endOfSize = raw.find("\r\n", pos);
        if (endOfSize == std::string::npos)
            throw std::runtime_error("Incomplete chunked body (no CRLF after size)");

        // Parse hex chunk size (ignore any extensions after ;)
        std::string sizeStr = raw.substr(pos, endOfSize - pos);
        size_t semi = sizeStr.find(';');
        if (semi != std::string::npos)
            sizeStr = sizeStr.substr(0, semi);

        // Trim spaces
        while (!sizeStr.empty() && std::isspace(sizeStr[0]))
            sizeStr.erase(sizeStr.begin());
        while (!sizeStr.empty() && std::isspace(sizeStr[sizeStr.size() - 1]))
            sizeStr.erase(sizeStr.end() - 1);

        if (sizeStr.empty())
            throw std::runtime_error("Empty chunk size line");

        size_t chunkSize = hexToSize(sizeStr);
        pos = endOfSize + 2; // skip CRLF

        if (chunkSize == 0)
        {
            // Final chunk — may have trailers, skip them
            size_t end = raw.find("\r\n\r\n", pos);
            // Some clients send only "\r\n" after 0 chunk
            if (end == std::string::npos)
                end = raw.size();
            break;
        }

        // Ensure we have enough data
        if (pos + chunkSize + 2 > raw.size())
            throw std::runtime_error("Incomplete chunk data");

        decoded.append(raw, pos, chunkSize);
        pos += chunkSize;

        // Expect CRLF after chunk data
        if (raw.compare(pos, 2, "\r\n") != 0)
            throw std::runtime_error("Missing CRLF after chunk data");
        pos += 2;
    }
    return decoded;
}


// ==== Constructor and Destructor ====
CGIHandler::CGIHandler(const std::string &scriptPath, const HTTPRequest &request, const LocationConfig &location)
	: _scriptPath(scriptPath), _cgiPath(location.cgiPath), _requestBody(request.GetBody())
	{
		std::cout << scriptPath << "<-  norm script path" << std::endl;
		std::cout << _scriptPath << "<- script path" << std::endl;
		_scriptPath = scriptPath;
		SetupEnvironment(request);
	}

CGIHandler::~CGIHandler() {}


// Sets up the environment variables for the CGI script
// void CGIHandler::SetupEnvironment(const HTTPRequest &request)
// {
// 	_envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
// 	_envVariables["SCRIPT_FILENAME"] = _scriptPath;
// 	_envVariables["REQUEST_METHOD"] = request.GetMethod();
// 	_envVariables["SERVER_PROTOCOL"] = request.GetHTTPVersion();
// 	_envVariables["CONTENT_LENGTH"] = request.GetHeaders().count("content-length") ? request.GetHeaders().find("content-length")->second : "0";
// 	_envVariables["CONTENT_TYPE"] = request.GetHeaders().count("content-type") ? request.GetHeaders().find("content-type")->second : "text/plain";
// 	_envVariables["QUERY_STRING"] = "";
// 	_envVariables["REDIRECT_STATUS"] = "200";
	
// }

void CGIHandler::SetupEnvironment(const HTTPRequest &request)
{
    _envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
    _envVariables["SCRIPT_FILENAME"] = _scriptPath;
    _envVariables["REQUEST_METHOD"] = request.GetMethod();
    _envVariables["SERVER_PROTOCOL"] = request.GetHTTPVersion();

    // Use the provided Content-Type or default
    std::map<std::string, std::string> headers = request.GetHeaders();
    if (headers.count("content-type"))
        _envVariables["CONTENT_TYPE"] = headers.find("content-type")->second;
    else
        _envVariables["CONTENT_TYPE"] = "text/plain";

    // Use the actual decoded body size
    std::ostringstream oss;
    oss << _requestBody.size();
    _envVariables["CONTENT_LENGTH"] = oss.str();

	// std::cout << "request body size: " << _requestBody.size();

    _envVariables["QUERY_STRING"] = "";
    _envVariables["REDIRECT_STATUS"] = "200";
}


std::string CGIHandler::BuildEnvString(const std::string &key, const std::string &value) { return (key + "=" + value); }

std::string CGIHandler::Execute()
{
	int pipein[2];
	int pipeout[2];
	if (pipe(pipein) == -1 || pipe(pipeout) == -1) // Create pipes
		throw std::runtime_error("CGI | Failed to create pipes");
	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("CGI | Failed to fork process");
	if (pid == 0) // Child process
	{
		dup2(pipein[0], STDIN_FILENO); // Redirect stdin
		dup2(pipeout[1], STDOUT_FILENO); // Redirect stdout
		
		close(pipein[1]);
		close(pipeout[0]);

		std::vector<std::string> environmentValues;
		for (std::map<std::string, std::string>::iterator it = _envVariables.begin(); it != _envVariables.end(); ++it)
			environmentValues.push_back(BuildEnvString(it->first, it->second));
		std::cout << "script path " << _scriptPath << std::endl;
		
		std::vector<char*> envp; // Prepare environment variables
		for (size_t i = 0; i < environmentValues.size(); ++i)
			envp.push_back(const_cast<char*>(environmentValues[i].c_str()));
		envp.push_back(NULL);
		
		char *argv[3]; // Prepare arguments
		argv[0] = const_cast<char*>(_cgiPath.c_str());
		argv[1] = const_cast<char*>(_scriptPath.c_str());
		argv[2] = NULL;

		execve(_cgiPath.c_str(), argv, &envp[0]); // Execute CGI script
		exit(1); // execve failed
	}

	// Parent process
	close(pipein[0]);
	close(pipeout[1]);
	if (!_requestBody.empty()) // Send request body to CGI
		write(pipein[1], _requestBody.c_str(), _requestBody.size());
	close(pipein[1]);

	char buffer[4096];
	ssize_t bytesRead;
	_cgiOutput.clear();
	while ((bytesRead = read(pipeout[0], buffer, sizeof(buffer))) > 0) // Read CGI output
		_cgiOutput.append(buffer, bytesRead);
	close(pipeout[0]);
	std::cout << "cgi output: " << _cgiOutput <<std::endl;
	waitpid(pid, NULL, 0); // Wait for child process to finish
	return (_cgiOutput);
}

