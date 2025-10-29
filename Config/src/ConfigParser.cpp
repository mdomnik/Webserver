/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:38:44 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 23:36:56 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"

// ==== Constructor ====
ConfigParser::ConfigParser(const std::string &filePath) : _filePath(filePath), _index(0)
{
	GetContent();
	TokenizeFile();
}

// ==== Private Methods ====

// Reads the content of a file into _fileContent
void ConfigParser::GetContent()
{
	int fd = open(_filePath.c_str(), O_RDONLY);
	if (fd < 0)
		throw std::runtime_error("config file could not be opened: " + _filePath);

	struct stat fileStat;
	if (fstat(fd, &fileStat) < 0)
	{
		close(fd);
		throw std::runtime_error("could not get stats from config file: " + _filePath);
	}

	_fileContent.clear();

	char buffer[5000];
	ssize_t bytesRead;
	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
	{
		_fileContent.append(buffer, bytesRead);
	}

	if (bytesRead < 0)
	{
		close(fd);
		throw std::runtime_error("error occured whilst reading config file: " + _filePath);
	}

	close(fd);
}

// Tokenizes the _fileContent into _tokens vector
void ConfigParser::TokenizeFile()
{
	std::string buffer;	 // buffer for building tokens
	int isInComment = 0; // checking if a line is a comment

	for (size_t i = 0; i < _fileContent.size(); ++i) // loop through each character
	{
		char c = _fileContent[i];

		if (isInComment)
		{
			if (c == '\n')
				isInComment = 0;
			continue;
		}
		if (c == '#')
		{
			isInComment = 1;
			continue;
		}
		if (std::isspace(static_cast<unsigned char>(c))) // whitespace
		{
			if (!buffer.empty())
			{
				_tokens.push_back(buffer);
				buffer.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';') // check if special char
		{
			if (!buffer.empty())
			{
				_tokens.push_back(buffer);
				buffer.clear();
			}
			_tokens.push_back(std::string(1, c));
		}
		else
		{
			buffer += c;
		}
	}
	if (!buffer.empty())
	{
		_tokens.push_back(buffer);
		buffer.clear();
	}
}

// ==== Token Methods ====

// Shows the next value in _tokens without advancing the index
std::string ConfigParser::Peek() const
{
	if (_index >= _tokens.size()) // if at end of tokens
		throw std::runtime_error("unexpected end of configuration file");
	return _tokens[_index];
}

// Returns the next value in _tokens and advances the index
std::string ConfigParser::Next()
{
	if (_index >= _tokens.size())
		throw std::runtime_error("unexpected end of configuration file");
	return _tokens[_index++];
}

// Confirms that the next token matches the expected value
void ConfigParser::Expect(const std::string &expected)
{
	std::string token = Next();
	if (token != expected)
		throw std::runtime_error("expected the token: " + expected + " got: " + token);
}

// ==== Block Parsing Methods ====

void initializeServerConfig(ServerConfig *serv) {
	serv->host = "0.0.0.0";
	serv->port = 8080;
	serv->serverName = "Default";
	serv->clientMaxBodySize = 1048576;
	serv->errorPages.clear();
	serv->locations.clear();
}
// Parses a server block and returns a ServerConfig object
ServerConfig ConfigParser::ParseServer()
{
	// Values need to be initialized to default values to avoid warnings!
	// ServerConfig serv = initializeServerConfig();
	ServerConfig serv;
	initializeServerConfig(&serv);

	Expect("server");
	Expect("{");

	while (Peek() != "}")
	{
		std::string token = Next();
		ParseServerParts(serv, token);
	}
	Expect("}");
	return (serv);
}

// Parse helper, tons of if-else to parse server parts
	// Uninitialized values stem here
void ConfigParser::ParseServerParts(ServerConfig &server, const std::string &token)
{
	if (token == "listen")
	{
		std::string ipaddress = Next();
		size_t delim = ipaddress.find(':');
		if (delim != std::string::npos)
		{
			server.host = ipaddress.substr(0, delim);
			server.port = std::atoi(ipaddress.substr(delim + 1).c_str());
		}
		else
			server.port = std::atoi(ipaddress.c_str());
		Expect(";");
	}
	else if (token == "client_max_body_size")
	{
		server.clientMaxBodySize = std::atoi(Next().c_str());
		Expect(";");
	}
	else if (token == "server_name")
	{
		server.serverName = Next();
		Expect(";");
	}
	else if (token == "error_page")
	{
		std::string pageName = Next();
		int code = std::atoi(pageName.c_str());
		pageName = Next();
		server.errorPages[code] = pageName;
		Expect(";");
	}
	else if (token == "location")
	{
		LocationConfig loc = ParseLocation();
		server.locations.push_back(loc);
	}
	else
		throw std::runtime_error("Unexpected token: " + token);
}

void initializeLocationConfig(LocationConfig *loc)
{
	loc->autoIndex = false;
	loc->index = "";
	loc->root = "";
	loc->path = "";
	loc->uploadStore = "";
	loc->redirection = "";
	loc->cgiExtention = "";
	loc->methods.clear();
}

// Parses a location block and returns a LocationConfig object
LocationConfig ConfigParser::ParseLocation()
{
	LocationConfig loc;
	initializeLocationConfig(&loc);
	loc.path = Next();
	Expect("{");

	while (Peek() != "}")
	{
		std::string token = Next();
		ParseLocationParts(loc, token);
	}
	Expect("}");
	return (loc);
}

// Parse helper, tons of if-else to parse location parts
void ConfigParser::ParseLocationParts(LocationConfig &location, const std::string &token)
{
	if (token == "root")
	{
		location.root = Next();
		Expect(";");
	}
	else if (token == "autoindex")
	{
		std::string value = Next();
		if (value == "on")
			location.autoIndex = true;
		else if (value == "off")
			location.autoIndex = false;
		else
			throw std::runtime_error("Wrong autoIndex value: " + value);
		Expect(";");
	}
	else if (token == "index")
	{
		location.index = Next();
		Expect(";");
	}
	else if (token == "methods")
	{
		while (Peek() != ";")
			location.methods.push_back(Next());
		Expect(";");
	}
	else if (token == "upload_store")
	{
		location.uploadStore = Next();
		Expect(";");
	}
	else if (token == "return")
	{
		location.redirection = Next();
		Expect(";");
	}
	else if (token == "cgi_extension")
	{
		location.cgiExtention = Next();
		Expect(";");
	}
	else
		throw std::runtime_error("Unexpected token: " + token);
}

// ==== Validation Method ====

// Validates the parsed ServerConfig for correctness
void ConfigParser::ValidateConfig(const ServerConfig &server)
{
	if (server.port < 0 || server.port > 65535)
		throw std::runtime_error("Invalid port number");
	if (server.clientMaxBodySize == 0)
		throw std::runtime_error("client_max_body_size must be greater than 0");
	if (server.locations.empty())
		throw std::runtime_error("At least one location block is required");

	for (size_t i = 0; i < server.locations.size(); ++i)
	{
		const LocationConfig &loc = server.locations[i];
		if (loc.path.empty())
			throw std::runtime_error("Location path cannot be empty");
		if (loc.root.empty())
			throw std::runtime_error("Location root cannot be empty for path: " + loc.path);
		if (!loc.redirection.empty() && !loc.cgiExtention.empty())
			throw std::runtime_error("Location cannot have both redirection and cgi_extension set for path: " + loc.path);

		std::set<std::string> validMethods;
		validMethods.insert("GET");
		validMethods.insert("POST");
		validMethods.insert("DELETE");

		for (size_t j = 0; j < loc.methods.size(); ++j)
		{
			if (validMethods.find(loc.methods[j]) == validMethods.end())
				throw std::runtime_error("Invalid method " + loc.methods[j] + " in location: " + loc.path);
		}

		if (loc.autoIndex && !loc.index.empty())
			std::cerr << "Warning: both autoindex and index set for location: " << loc.path << std::endl;

		if (!loc.cgiExtention.empty())
			if (loc.cgiExtention[0] != '.')
				throw std::runtime_error("CGI extension must start with a dot in location: " + loc.path);
	}
}

// ==== Public Parse Method ====
std::vector<ServerConfig> ConfigParser::parse()
{
	std::vector<ServerConfig> servers;
	_index = 0;

	while (_index < _tokens.size())
	{
		std::string token = Peek();
		if (token == "server")
			servers.push_back(ParseServer());
		else
			throw std::runtime_error("expected the token: server got: " + token);
	}
	for (size_t i = 0; i < servers.size(); i++)
		ValidateConfig(servers[i]);
	return (servers);
}