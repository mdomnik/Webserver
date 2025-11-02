/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:26:14 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 18:47:26 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ServerConfig.hpp"
#include <string>
#include <vector>
#include <set>
#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>
#include <algorithm>
#include <list>

#define MAX_CLIENT_BODY_SIZE 10485760

class ConfigParser
{
	private:
		std::string _filePath; // path to config file
		std::string _fileContent; // read contents of config file
		std::vector<std::string> _tokens; // tokenized content
		size_t _index;

		// internal parsing methods
		void GetContent();
		void TokenizeFile();

		// Token methods
		std::string Peek() const;
		std::string Next();
		void Expect(const std::string& expected);

		// block parsing methods
		ServerConfig ParseServer();
		LocationConfig ParseLocation();

		// Parse Helpers for if statements
		void ParseServerParts(ServerConfig& server, const std::string& token);
		void ParseLocationParts(LocationConfig& loc, const std::string& token);

		// Check for completion
		void ValidateConfig(ServerConfig& server);

	public:
		// Constructor
		ConfigParser(const std::string& filePath);
		
		// Main parse method
		std::vector<ServerConfig> parse();

};

#endif