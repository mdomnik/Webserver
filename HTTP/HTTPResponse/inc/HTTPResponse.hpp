/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:11:20 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/04 13:26:58 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "../../Config/inc/ServerConfig.hpp"
#include "../../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../../../Server/inc/Server.hpp"
#include "../../Config/inc/ConfigAnsi.hpp"

class HTTPResponse
{
	private:
		int		ourLittleSecret;
		std::string		_statusLine;
		std::map<std::string, std::string> _headers;
		std::string _body;
		
		
		// utility methods
		std::string GetFileType(const std::string &path);
		bool IsDirectory(const std::string &path);
		bool IsFile(const std::string &path);
		bool IsEndOfString(const std::string &str, const std::string &endpart);
		std::string buildAutoIndexPage(const std::string &path, const std::string &uri);
		bool IsMethodAllowed(const LocationConfig &location, const std::string &method, const std::string &path);
		
		// Method handlers
		std::string HandleGET(const HTTPRequest &req, const ServerConfig &conf);
		std::string HandlePOST(const HTTPRequest &req, const ServerConfig &conf);
		std::string HandleDELETE(const HTTPRequest &req, const ServerConfig &conf);
		
		std::string ResponseFromCGI(const std::string &cgiOutput, const std::string &httpVersion);
		std::string LoadErrorPage(int statusCode, const ServerConfig &config);
		
		public:
		// Constructor
		HTTPResponse();
		
		//setters
		void SetStatus(int statusCode, const std::string &httpVersion, const std::string &reason);
		void SetHeader(const std::string &key, const std::string &value);
		void SetBody(const std::string &body);
		// Response generation
		std::string GenerateResponse(const HTTPRequest &request, const ServerConfig &config);
		std::string ResponseToString() const;
		void SetResponseToError(int code, const std::string &version, const std::string &reason, const ServerConfig& config);
		
		// Getters
		std::string GetStatus() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;
		void RemoveHeader(const std::string &key);
	};
	
	const LocationConfig& FindMostMatchingLocation(const ServerConfig &serverConfig, const std::string &requestPath);
	#endif