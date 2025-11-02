/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 20:13:05 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 22:46:32 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/CGIHandler.hpp"

// ==== Constructor and Destructor ====
CGIHandler::CGIHandler(const std::string &scriptPath, const HTTPRequest &request, const LocationConfig &location)
	: _scriptPath(scriptPath), _cgiPath(location.cgiPath), _requestBody(request.GetBody())
	{
		SetupEnvironment(request);
	}

CGIHandler::~CGIHandler() {}


// Sets up the environment variables for the CGI script
void CGIHandler::SetupEnvironment(const HTTPRequest &request)
{
	_envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
	_envVariables["SCRIPT_FILENAME"] = _scriptPath;
	_envVariables["REQUEST_METHOD"] = request.GetMethod();
	_envVariables["SERVER_PROTOCOL"] = request.GetHTTPVersion();
	_envVariables["CONTENT_LENGTH"] = request.GetHeaders().count("content-length") ? request.GetHeaders().find("content-length")->second : "0";
	_envVariables["CONTENT_TYPE"] = request.GetHeaders().count("content-type") ? request.GetHeaders().find("content-type")->second : "text/plain";
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
		std::cout << "script path: " << _scriptPath << std::endl; // this fix testes with cgi . COME BACK TO IT LATER
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
	
	waitpid(pid, NULL, 0); // Wait for child process to finish
	return (_cgiOutput);
}