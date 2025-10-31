/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponseMethods.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:24:16 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/31 15:30:28 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPResponse.hpp"
#include "../../CGI/inc/CGIHandler.hpp"

std::string HTTPResponse::HandleGET(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	std::string path = req.GetPath();
	const LocationConfig &location = config.locations[0];
	std::string root = config.locations.empty() ? "./www" : location.root;
	std::string fullPath = root + path;

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
					return (ResponseToString());
				}
				SetResponseToError(404, version, "Not Found");
				return (ResponseToString());
			}
			return ResponseFromCGI(cgiOutput, version);
		}
		catch(const std::exception& e)
		{
			std::cerr << "CGI Execution Error: " << e.what() << std::endl;
			SetResponseToError(500, version, "Internal Server Error");
			return (ResponseToString());
		}
		
	}
	
	if (IsDirectory(fullPath))
	{
		const LocationConfig &location = config.locations[0];
		if (location.autoIndex)
		{
			SetStatus(200, version, "OK");
			SetHeader("Content-Type", "text/html");
			SetBody(buildAutoIndexPage(fullPath, path));
			return (ResponseToString());
		}
		fullPath += "/" + location.index;
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
			SetResponseToError(404, version, "Not Found");
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
			SetResponseToError(403, version, "Forbidden");
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
	const LocationConfig &location = config.locations[0];
	std::string root = config.locations.empty() ? "./www" : location.root;
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
					SetResponseToError(404, version, "Not Found");
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
				SetResponseToError(500, version, "Internal Server Error");
			return (ResponseToString());
		}
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
			SetResponseToError(403, version, "Forbidden");
		return (ResponseToString());
	}
	
	std::string destanation = location.uploadStore + "/upload.txt";
	std::ofstream outFile(destanation.c_str(), std::ios::out | std::ios::binary);
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
			SetResponseToError(500, version, "Internal Server Error");
		return (ResponseToString());
	}
	outFile << req.GetBody();
	outFile.close();

	SetStatus(201, version, "Created");
	SetHeader("Content-Type", "text/html");
	SetBody("File uploaded successfully to " + destanation);

	return (ResponseToString());
}

std::string HTTPResponse::HandleDELETE(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	std::string root = config.locations.empty() ? "./www" : config.locations[0].root;
	std::string fullPath = root + req.GetPath();

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
			SetResponseToError(404, version, "Not Found");
		return (ResponseToString());
	}

	SetStatus(200, version, "OK");
	SetHeader("Content-Type", "text/html");
	SetBody("File deleted successfully.");
	return (ResponseToString());
}

