/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponseMethods.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:24:16 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 22:43:24 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPResponse.hpp"
#include "../../CGI/inc/CGIHandler.hpp"

static std::string URLDecode(const std::string &src)
{
    std::string decoded;
    char hex[3] = {0};
    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src[i] == '%' && i + 2 < src.size())
        {
            hex[0] = src[i + 1];
            hex[1] = src[i + 2];
            decoded += static_cast<char>(strtol(hex, NULL, 16));
            i += 2;
        }
        else if (src[i] == '+')
            decoded += ' ';
        else
            decoded += src[i];
    }
	// std::cout << "Decoded URL: " << decoded << std::endl;
    return decoded;
}

std::string HTTPResponse::HandleGET(const HTTPRequest &req, const ServerConfig &config)
{
	std::string version = req.GetHTTPVersion().empty() ? "HTTP/1.1" : req.GetHTTPVersion();
	std::string path = URLDecode(req.GetPath());
	const LocationConfig &location = FindMostMatchingLocation(config, path);
	std::string root = location.root.empty() ? "./www" : location.root;
	std::cout << "Location root: " << root << " and path " << path << std::endl;
	std::string fullPath = root + path;
	std::cout << "Handling GET for path: " << fullPath << std::endl;

	if (!location.cgiExtension.empty() && fullPath.size() >= location.cgiExtension.size() && fullPath.substr(fullPath.size() - location.cgiExtension.size()) == location.cgiExtension)
	{
		try
		{
			CGIHandler cgi(fullPath, req, location);
			std::string cgiOutput = cgi.Execute();
			// std::cout << cgiOutput << std::endl;
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
				SetResponseToError(404, version, "Not Found");
				return (ResponseToString());
			}
			std::cout << "CGI executed successfully." << std::endl;
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
		std::cout << "Path is a directory." << std::endl;
		const LocationConfig &location = FindMostMatchingLocation(config, path);
		if (location.index.empty())
		{
			SetStatus(200, version, "OK");
			SetHeader("Content-Type", "text/html");
			SetBody(buildAutoIndexPage(fullPath, path));
			return (ResponseToString());
		}
		if (!location.index.empty())
			fullPath += "/" + location.index;
		std::cout << "Index file path: " << fullPath << std::endl;
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
			SetResponseToError(500, version, "Internal Server Error");
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

