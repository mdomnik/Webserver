/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 20:13:05 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/04 15:41:12 by mdomnik          ###   ########.fr       */
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
pid_t	CGIHandler::GetCGIPid(){
	return _cgiPid;
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


int CGIHandler::StartCGI()
{
	int sv[2];

	// Create non-blocking bidirectional socketpair
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, sv) == -1)
		throw std::runtime_error("CGI | Failed to create socketpair");

	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("CGI | Failed to fork process");

	if (pid == 0)
	{
		// --- Child process (CGI script) ---
		close(sv[0]); // Close parent end

		dup2(sv[1], STDIN_FILENO);
		dup2(sv[1], STDOUT_FILENO);
		dup2(sv[1], STDERR_FILENO);
		close(sv[1]);

		// Build environment
		std::vector<std::string> envStrings;
		for (std::map<std::string, std::string>::iterator it = _envVariables.begin();
			 it != _envVariables.end(); ++it)
		{
			envStrings.push_back(it->first + "=" + it->second);
		}

		std::vector<char*> envp;
		for (size_t i = 0; i < envStrings.size(); ++i)
			envp.push_back(const_cast<char*>(envStrings[i].c_str()));
		envp.push_back(NULL);

		// Build argv
		char *argv[3];
		argv[0] = const_cast<char*>(_cgiPath.c_str());   // interpreter, e.g. /usr/bin/python3
		argv[1] = const_cast<char*>(_scriptPath.c_str()); // actual script path
		argv[2] = NULL;

		execve(_cgiPath.c_str(), argv, &envp[0]);
		perror("execve");
		_exit(1);
	}

	// --- Parent process ---
	close(sv[1]);	 // close child end
	_cgiPid = pid;	// store PID for later cleanup

	// --- Write POST data if any ---
	std::cerr << "Writing POST body (" << _requestBody.size() << " bytes)\n";

	if (!_requestBody.empty()) {
		size_t total_written = 0;
		while (total_written < _requestBody.size()) {
			ssize_t n = write(sv[0], _requestBody.data() + total_written,
							  _requestBody.size() - total_written);
			if (n > 0) total_written += n;
			else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
				usleep(1000);
				continue;
			} else break;
		}
	}
	shutdown(sv[0], SHUT_WR);
	
	return sv[0];
}
