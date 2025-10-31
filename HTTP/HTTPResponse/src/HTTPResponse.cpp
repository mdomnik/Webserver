/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:11:35 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/31 15:26:08 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPResponse.hpp"

// ==== Constructor ====
HTTPResponse::HTTPResponse() {}

// ==== Getters ====
std::string HTTPResponse::GetStatus() const { return (_statusLine); }
std::map<std::string, std::string> HTTPResponse::GetHeaders() const { return (_headers); }
std::string HTTPResponse::GetBody() const { return (_body); }

// ==== Setters ====
void HTTPResponse::SetStatus(int statusCode, const std::string &httpVersion, const std::string &reason)
{
	std::ostringstream status;
	status << httpVersion << " " << statusCode << " " << reason;
	_statusLine = status.str();
}

void HTTPResponse::SetHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void HTTPResponse::SetBody(const std::string &body)
{
	_body = body;
	std::ostringstream contentLength; //also update Content-Length header
	contentLength << body.size();
	_headers["Content-Length"] = contentLength.str();
}

// ==== Response Generation ====

// Converts the HTTPResponse object to a raw HTTP response string
std::string HTTPResponse::ResponseToString() const
{
	std::ostringstream response;
	response << _statusLine << CRLF;
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << CRLF;
	response << CRLF; // blank line between headers and body
	response << _body;
	return (response.str());
}

// Generates the HTTP response based on the request and server configuration
std::string HTTPResponse::GenerateResponse(const HTTPRequest &request, const ServerConfig &config)
{
	std::string method = request.GetMethod();
	if (method == "GET")
	{
		return (HandleGET(request, config));
	}
	else if (method == "POST")
	{
		return (HandlePOST(request, config));
	}
	else if (method == "DELETE")
	{
		return (HandleDELETE(request, config));
	}
	else
	return (SetResponseToError(405, request.GetHTTPVersion(), "Method Not Allowed"), ResponseToString());
}

std::string HTTPResponse::LoadErrorPage(int statusCode, const ServerConfig &config)
{
	std::map<int, std::string>::const_iterator it = config.errorPages.find(statusCode);
	if (it != config.errorPages.end())
	{
		std::string path;
		if (config.locations.empty())
		{
			path = "." + it->second;
		}
		else
		{
			path = config.locations[0].root + it->second;
		}
		if (IsFile(path))
		{
			std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
			if (file.is_open())
			{
				std::ostringstream fileContent;
				fileContent << file.rdbuf();
				file.close();
				return (fileContent.str());
			}
		}
	}
	return ("");
}

void HTTPResponse::SetResponseToError(int code, const std::string &version, const std::string &reason)
{
	std::ostringstream body;
	body << "<html><head><title>" << code << " " << reason << "</title></head>"
		 << "<body><h1>" << code << " " << reason << "</h1></body></html>";
	SetStatus(code, version, reason);
	SetHeader("Content-Type", "text/html");
	SetBody(body.str());
}