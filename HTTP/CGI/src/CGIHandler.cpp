/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 20:13:05 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/04 13:55:50 by fjoestin         ###   ########.fr       */
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


// std::string CGIHandler::Execute(int &has_failed)
// {
//     int pipein[2];
//     int pipeout[2];
//     if (pipe(pipein) == -1 || pipe(pipeout) == -1)
//         throw std::runtime_error("CGI | Failed to create pipes");

//     pid_t pid = fork();
//     if (pid < 0)
//         throw std::runtime_error("CGI | Failed to fork process");

//     if (pid == 0)
//     {
//         dup2(pipein[0], STDIN_FILENO);
//         dup2(pipeout[1], STDOUT_FILENO);
//         close(pipein[1]);
//         close(pipeout[0]);

//         std::vector<std::string> envStrings;
//         for (std::map<std::string, std::string>::iterator it = _envVariables.begin();
//              it != _envVariables.end(); ++it)
//             envStrings.push_back(it->first + "=" + it->second);

//         std::vector<char*> envp;
//         for (size_t i = 0; i < envStrings.size(); ++i)
//             envp.push_back(const_cast<char*>(envStrings[i].c_str()));
//         envp.push_back(NULL);

//         char *argv[3];
//         argv[0] = const_cast<char*>(_cgiPath.c_str());
//         argv[1] = const_cast<char*>(_scriptPath.c_str());
//         argv[2] = NULL;

//         execve(_cgiPath.c_str(), argv, &envp[0]);
//         exit(1);
//     }

//     // --- Parent process ---
//     close(pipein[0]);
//     close(pipeout[1]);
//     if (!_requestBody.empty())
//         write(pipein[1], _requestBody.c_str(), _requestBody.size());
//     close(pipein[1]);

//     // Make the CGI output pipe nonblocking
//     fcntl(pipeout[0], F_SETFL, O_NONBLOCK);

//     _cgiOutput.clear();
//     char buffer[4096];
//     int status = 0;
//     time_t start = time(NULL);

//     // We’ll poll the pipe repeatedly but yield periodically to avoid freezing the loop
//     while (true)
//     {
//         ssize_t bytes = read(pipeout[0], buffer, sizeof(buffer));
//         if (bytes > 0)
//         {
//             _cgiOutput.append(buffer, bytes);
//         }
//         else if (bytes == 0)
//         {
//             // EOF
//             break;
//         }
//         else if (errno == EAGAIN || errno == EWOULDBLOCK)
//         {
//             // No data right now — yield briefly to let other events run
//             usleep(2000); // 2ms pause keeps CPU low but loop responsive
//         }
//         else
//         {
//             // Some read error
//             break;
//         }

//         // Non-blocking waitpid (checks if child exited)
//         int ret = waitpid(pid, &status, WNOHANG);
//         if (ret > 0)
//             break;

//         // Timeout safety
//         if (time(NULL) - start > 5)
//         {
//             kill(pid, SIGKILL);
//             waitpid(pid, &status, 0);
//             has_failed = 1;
//             throw std::runtime_error("CGI | Script timed out");
//         }
//     }

//     close(pipeout[0]);

//     if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
//         has_failed = 1;

//     return _cgiOutput;
// }




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
    close(sv[1]);     // close child end
    _cgiPid = pid;    // store PID for later cleanup

    // --- Write POST data if any ---
    if (!_requestBody.empty())
    {
        size_t total_written = 0;
        while (total_written < _requestBody.size())
        {
            ssize_t n = write(sv[0], _requestBody.data() + total_written,
                              _requestBody.size() - total_written);
            if (n > 0)
            {
                total_written += n;
            }
            else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
				usleep(1000);
				continue;
            }
            else
            {
                perror("CGI | write request body");
                break;
            }
        }

        // Optional: if partial write occurred, you can track remaining data
        if (total_written < _requestBody.size()) {
            _remainingBody = _requestBody.substr(total_written);
        } else {
            _remainingBody.clear();
        }
    }

    return sv[0]; // return non-blocking socket FD for epoll
}






// std::string CGIHandler::Execute(int &has_failed)
// {
//     int sv[2];
//     if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, sv) == -1)
//         throw std::runtime_error("CGI | Failed to create socketpair");

//     pid_t pid = fork();
//     if (pid < 0)
//         throw std::runtime_error("CGI | Failed to fork process");

//     if (pid == 0)
//     {
//         // --- Child process (CGI script) ---
//         close(sv[0]); // Close parent end

//         // Redirect CGI’s stdin and stdout to the socket
//         dup2(sv[1], STDIN_FILENO);
//         dup2(sv[1], STDOUT_FILENO);
//         close(sv[1]);

//         // Build environment variables
//         std::vector<std::string> envStrings;
//         for (std::map<std::string, std::string>::iterator it = _envVariables.begin();
//              it != _envVariables.end(); ++it)
//             envStrings.push_back(it->first + "=" + it->second);

//         std::vector<char*> envp;
//         for (size_t i = 0; i < envStrings.size(); ++i)
//             envp.push_back(const_cast<char*>(envStrings[i].c_str()));
//         envp.push_back(NULL);

//         char *argv[3];
//         argv[0] = const_cast<char*>(_cgiPath.c_str());
//         argv[1] = const_cast<char*>(_scriptPath.c_str());
//         argv[2] = NULL;

//         execve(_cgiPath.c_str(), argv, &envp[0]);
//         perror("execve");
//         _exit(1);
//     }

//     // --- Parent process ---
//     close(sv[1]); // Close child end

//     // Send request body (if POST)
//     if (!_requestBody.empty())
//     {
//         size_t total_written = 0;
//         while (total_written < _requestBody.size())
//         {
//             ssize_t n = write(sv[0], _requestBody.data() + total_written,
//                               _requestBody.size() - total_written);
//             if (n > 0)
//                 total_written += n;
//             else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
//             {
//                 usleep(2000); // Small backoff — or use poll() in your main loop
//                 continue;
//             }
//             else
//             {
//                 perror("CGI | write");
//                 break;
//             }
//         }
//     }

//     // --- Read CGI output ---
//     _cgiOutput.clear();
//     char buffer[4096];
//     int status = 0;
//     time_t start = time(NULL);

//     while (true)
//     {
//         ssize_t bytes = read(sv[0], buffer, sizeof(buffer));
//         if (bytes > 0)
//         {
//             _cgiOutput.append(buffer, bytes);
//         }
//         else if (bytes == 0)
//         {
//             // EOF
//             break;
//         }
//         else if (errno == EAGAIN || errno == EWOULDBLOCK)
//         {
//             // No data yet — yield to avoid busy loop
//             usleep(2000);
//         }
//         else
//         {
//             perror("CGI | read");
//             break;
//         }

//         // Check if CGI exited
//         int ret = waitpid(pid, &status, WNOHANG);
//         if (ret > 0)
//             break;

//         // Timeout safety
//         if (time(NULL) - start > 5)
//         {
//             kill(pid, SIGKILL);
//             waitpid(pid, &status, 0);
//             has_failed = 1;
//             throw std::runtime_error("CGI | Script timed out");
//         }
//     }

//     close(sv[0]);

//     if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
//         has_failed = 1;

//     return _cgiOutput;
// }
