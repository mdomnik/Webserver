/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 20:13:07 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 21:55:02 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#include "../../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../../../Config/inc/ServerConfig.hpp"

class CGIHandler
{
	private:
		std::string _scriptPath;
		std::string _cgiPath;
		std::map<std::string, std::string> _envVariables;
		std::string _requestBody;
		std::string _cgiOutput;

		void SetupEnvironment(const HTTPRequest &request);
		std::string BuildEnvString(const std::string &key, const std::string &value);
	public:
		CGIHandler(const std::string &scriptPath, const HTTPRequest &request, const LocationConfig &location);
		~CGIHandler();
		
		std::string Execute();
};

std::string decodeChunkedBody(const std::string &raw);

#endif