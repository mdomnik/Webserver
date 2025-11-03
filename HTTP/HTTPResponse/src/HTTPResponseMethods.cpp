/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponseMethods.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:24:16 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/03 15:54:29 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPResponse.hpp"
#include "../../CGI/inc/CGIHandler.hpp"
#include "../inc/ConfigAnsi.hpp"
#include <string>
#include <algorithm>

std::string HTTPResponse::HandleGET(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	std::string path = req.GetPath();
	const LocationConfig &location = FindMostMatchingLocation(config, path);
	std::string root = location.root.empty() ? "./www" : location.root;
	std::cout << "Location root: " << root << " and path " << path << std::endl;
	std::string fullPath = root + path;

	if (std::find(location.methods.begin(), location.methods.end(), "GET") == location.methods.end())
	{
		return (SetResponseToError(405, version, "Method Not Allowed", config), ResponseToString());
	}

	std::cerr << MAGENTA << "Handling GET for path: " << fullPath << ESCAPE << std::endl;

	if (!location.cgiExtension.empty() && fullPath.size() >= location.cgiExtension.size() && fullPath.substr(fullPath.size() - location.cgiExtension.size()) == location.cgiExtension)
	{
		try
		{
			CGIHandler cgi(fullPath, req, location);
			std::string cgiOutput = cgi.Execute();
			std::cout << "cgioutput content: " << cgiOutput << std::endl;
			std::cout << "cgioutput length: " << cgiOutput.size() << std::endl;
			if (cgiOutput.empty())
			{
				std::string customPage = LoadErrorPage(404, config);
				if (!customPage.empty())
				{
					std::cout << "CGI output empty, serving custom 404 page." << std::endl;
					SetStatus(404, version, "Not Found");
					SetHeader("Content-Type", "text/html");
					SetBody(customPage);
					return (ResponseToString());
				}
				SetResponseToError(404, version, "Not Found", config);
				return (ResponseToString());
			}
			std::cout << "CGI executed successfully." << std::endl;
			return ResponseFromCGI(cgiOutput, version);
		}
		catch(const std::exception& e)
		{
			std::cerr << "CGI Execution Error: " << e.what() << std::endl;
			SetResponseToError(500, version, "Internal Server Error", config);
			return (ResponseToString());
		}
		
	}
	
	if (IsDirectory(fullPath))
	{
		std::cout << YELLOW << "Requested path is a directory: " << fullPath << ESCAPE << std::endl;
		const LocationConfig &location = FindMostMatchingLocation(config, path);

		std::cout << CYAN << "Location root: " << location.root << ", index: " << location.index << ", autoIndex: " << (location.autoIndex ? "on" : "off") << ESCAPE << std::endl;
		if (!location.index.empty() && IsFile(fullPath + "/" + location.index))
		{
			fullPath += "/" + location.index;
			std::cout << GREEN << "Serving index file: " << fullPath << ESCAPE << std::endl;
		}
		else if (location.autoIndex)
		{
			std::cout << GREEN << "Autoindex is enabled for path: " << fullPath << ESCAPE << std::endl;
			SetStatus(200, version, "OK");
			SetHeader("Content-Type", "text/html");
			SetBody(buildAutoIndexPage(fullPath, path));
			return (ResponseToString());
		} else {
			SetResponseToError(404, version, "Not Found", config);
			return (ResponseToString());
		}
	}

	if (!IsFile(fullPath))
	{
		std::string customPage = LoadErrorPage(404, config);
		if (!customPage.empty())
		{
			SetStatus(404, version, "Not Found");
			SetHeader("Content-Type", "text/html");
			SetBody(customPage);
		}
		else
			SetResponseToError(404, version, "Not Found", config);
		return (ResponseToString());
	}

	std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::string customPage = LoadErrorPage(403, config);
		if (!customPage.empty())
		{
			SetStatus(403, version, "Forbidden");
			SetHeader("Content-Type", "text/html");
			SetBody(customPage);
		}
		else
			SetResponseToError(403, version, "Forbidden", config);
		return (ResponseToString());
	}

	std::ostringstream fileContent;
	fileContent << file.rdbuf();
	file.close();

	SetStatus(200, version, "OK");
	SetHeader("Content-Type", GetFileType(fullPath));
	SetBody(fileContent.str());
	return (ResponseToString());
}

std::string HTTPResponse::HandlePOST(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	const LocationConfig &location = FindMostMatchingLocation(config, req.GetPath());
	std::string root = location.root.empty() ? "./www" : location.root;
	std::string fullPath = root + req.GetPath();
	
	
	if (!location.cgiExtension.empty() && fullPath.size() >= location.cgiExtension.size() && fullPath.substr(fullPath.size() - location.cgiExtension.size()) == location.cgiExtension)
	{
		try
		{
			CGIHandler cgi(fullPath, req, location);
			std::string cgiOutput = cgi.Execute();
			if (cgiOutput.empty())
			{
				std::string customPage = LoadErrorPage(404, config);
				if (!customPage.empty())
				{
					SetStatus(404, version, "Not Found");
					SetHeader("Content-Type", "text/html");
					SetBody(customPage);
				}
				else
				SetResponseToError(404, version, "Not Found", config);
				return (ResponseToString());
			}
			return (ResponseFromCGI(cgiOutput, version));
		}
		catch(const std::exception& e)
		{
			std::cerr << "CGI Execution Error: " << e.what() << std::endl;
			std::string customPage = LoadErrorPage(500, config);
			if (!customPage.empty())
			{
				SetStatus(500, version, "Internal Server Error");
				SetHeader("Content-Type", "text/html");
				SetBody(customPage);
			}
			else
			SetResponseToError(500, version, "Internal Server Error", config);
			return (ResponseToString());
		}
	}
	
	if (std::find(location.methods.begin(), location.methods.end(), "POST") == location.methods.end())
	{
		return (SetResponseToError(405, version, "Method Not Allowed", config), ResponseToString());
	}
	if (!location.uploadEnable)
	{
		std::string customPage = LoadErrorPage(403, config);
		if (!customPage.empty())
		{
			SetStatus(403, version, "Forbidden");
			SetHeader("Content-Type", "text/html");
			SetBody(customPage);
		}
		else
			SetResponseToError(403, version, "Forbidden", config);
		return (ResponseToString());
	}
	
	std::string destination = location.uploadStore + "/upload.txt";
	std::ofstream outFile(destination.c_str(), std::ios::out | std::ios::binary);
	if (!outFile.is_open())
	{
		std::string customPage = LoadErrorPage(500, config);
		if (!customPage.empty())
		{
			SetStatus(500, version, "Internal Server Error");
			SetHeader("Content-Type", "text/html");
			SetBody(customPage);
		}
		else
			SetResponseToError(500, version, "Internal Server Error", config);
		return (ResponseToString());
	}
	outFile << req.GetBody();
	outFile.close();

	SetStatus(201, version, "Created");
	SetHeader("Content-Type", "text/html");
	SetBody("File uploaded successfully to " + destination);

	return (ResponseToString());
}

std::string HTTPResponse::HandleDELETE(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	const LocationConfig &location = FindMostMatchingLocation(config, req.GetPath());
	std::string root = location.root.empty() ? "./www" : location.root;
	std::string fullPath = root + req.GetPath();

	if (std::find(location.methods.begin(), location.methods.end(), "DELETE") == location.methods.end())
	{
		return (SetResponseToError(405, version, "Method Not Allowed", config), ResponseToString());
	}

	if (remove(fullPath.c_str()) != 0)
	{
		std::string customPage = LoadErrorPage(404, config);
		if (!customPage.empty())
		{
			SetStatus(404, version, "Not Found");
			SetHeader("Content-Type", "text/html");
			SetBody(customPage);
		}
		else
			SetResponseToError(404, version, "Not Found", config);
		return (ResponseToString());
	}

	SetStatus(200, version, "OK");
	SetHeader("Content-Type", "text/html");
	SetBody("File deleted successfully.");
	return (ResponseToString());
}

