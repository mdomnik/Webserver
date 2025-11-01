/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponseUtils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:17:44 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 21:08:01 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPResponse.hpp"

// Checks if a string ends with a specific substring
bool HTTPResponse::IsEndOfString(const std::string &str, const std::string &endpart)
{
	if (endpart.empty())
		return (false);
	if (str.compare(str.size() - endpart.size(), endpart.size(), endpart) == 0)
		return (true);
	return (false);
}

// Checks if the given path is a directory
bool HTTPResponse::IsDirectory(const std::string &path)
{
	struct stat stats;
	if (stat(path.c_str(), &stats) == 0)
	{
		if (S_ISDIR(stats.st_mode))
			return (true);
	}
		return (false);
}

// Checks if the given path is a regular file
bool HTTPResponse::IsFile(const std::string &path)
{
	struct stat stats;
	if (stat(path.c_str(), &stats) == 0)
	{
		if (S_ISREG(stats.st_mode))
			return (true);
	}
	return (false);
}

// Gets the MIME type of a file based on its extension
std::string HTTPResponse::GetFileType(const std::string &path)
{
	if (IsEndOfString(path, ".html") || IsEndOfString(path, ".htm"))
		return ("text/html");
	else if (IsEndOfString(path, ".css"))
		return ("text/css");
	else if (IsEndOfString(path, ".js"))
		return ("application/javascript");
	else if (IsEndOfString(path, ".jpg") || IsEndOfString(path, ".jpeg"))
		return ("image/jpeg");
	else if (IsEndOfString(path, ".png"))
		return ("image/png");
	else if (IsEndOfString(path, ".gif"))
		return ("image/gif");
	else if (IsEndOfString(path, ".txt"))
		return ("text/plain");
	else if (IsEndOfString(path, ".pdf"))
		return ("application/pdf");
	else
		return ("application/octet-stream"); // default binary type
}

std::string HTTPResponse::buildAutoIndexPage(const std::string &Path, const std::string &uri)
{
		std::ostringstream html;
	html << "<html><head><title>Index of " << uri << "</title></head><body>"; //html header
	
	DIR *dir = opendir(Path.c_str()); //open directory
	if (!dir)
		return ("<h1>403 Forbidden</h1>"); //if cannot open, return forbidden
	
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) //read each entry
	{
		std::string name = entry->d_name; //get entry name
		if (name == "." || name == "..") //skip current and parent directory
			continue;
		html << "<li><a href=\"" << name << "\">" << name << "</a></li>"; //add link to html
	}
	closedir(dir);

	html << "</ul></body></html>";
	return (html.str());
}

std::string HTTPResponse::ResponseFromCGI(const std::string &out, const std::string &httpversion)
{
	// Split CGI output into headers and body
	size_t pos = out.find(DOUBLECRLF);
	size_t length = 4;
	if (pos == std::string::npos)
	{
		pos = out.find("\n\n");
		length = 2;
	}

	std::string headerPart;
	std::string bodyPart;

	// if no headers found, treat entire output as body
	if (pos == std::string::npos)
	{
		headerPart = "";
		bodyPart = out;
	}
	else
	{
		headerPart = out.substr(0, pos);
		bodyPart = out.substr(pos + length);
	}

	// default values
	int status = 200; 
	std::string statusText = "OK";

	size_t i = 0; //parse headers
	while (i < headerPart.size())
	{
		size_t end = headerPart.find(CRLF, i); //find end of line
		size_t forward = 2; //default forward for CRLF
		if (end == std::string::npos) //if no CRLF found, try LF
		{
			end = headerPart.find('\n', i); //find end of line with LF
			if (end == std::string::npos) //if no LF found either, use entire remaining string
				end = headerPart.size();
			forward = (end < headerPart.size()) ? 1 : 0; //adjust forward for LF
		}

		std::string line = headerPart.substr(i, end - i); //isolate line
		i = (end < headerPart.size()) ? end + forward : headerPart.size(); //move iterator forward

		if (line.empty()) //if empty line, skip
			continue;
		
		if (line.size() > 7 && line.substr(0, 7) == "Status:") //if status line
		{
			std::string remain = line.substr(7); //get remaining part
			size_t k = 0;
			while (k < remain.size() && (remain[k] == ' ' || remain[k] == '\t')) //skip whitespace
				++k;
			remain = remain.substr(k);

			std::istringstream statusStream(remain);
			int code = 0;
			statusStream >> code; //extract status code
			if (code >= 100 && code <= 599) //validate code range
			{
				status = code;
				std::string after;
				std::getline(statusStream, after);
				if (!after.empty() && after[0] == ' ')
					after.erase(0, 1);
				if (!after.empty())
					statusText = after; //extract status text
			}
			continue;
		}

		// parse regular header line
		size_t colon = line.find(':'); //find delimiter
		if (colon == std::string::npos) 
			continue;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t z = 0; //trim whitespace from value
		while (z < value.size() && (value[z] == ' ' || value[z] == '\t'))
			++z;
		value = value.substr(z);
		
		if (!key.empty() && !value.empty())
			SetHeader(key, value); //set header
	}
	
		std::ostringstream l;
		l << bodyPart.size();
		SetHeader("content-length", l.str());
		
	SetStatus(status, httpversion, statusText);
	SetBody(bodyPart);
	return (ResponseToString());
}

const LocationConfig& HTTPResponse::FindMostMatchingLocation(const ServerConfig &serverConfig, const std::string &requestPath)
{
	const LocationConfig* bestMatch = &serverConfig.locations[0];
	
	size_t Length = 0;
	
	for (size_t i = 0; i < serverConfig.locations.size(); ++i)
	{
		const LocationConfig& loc = serverConfig.locations[i];
		if (requestPath.find(loc.path) == 0 && loc.path.size() > Length)
		{
			bestMatch = &loc;
			Length = loc.path.size();
		}
	}
	std::cout << "Finding location for path: " << bestMatch->path << std::endl;
	return (*bestMatch);
}

bool HTTPResponse::IsMethodAllowed(const LocationConfig &location, const std::string &method)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		return (false);
    if (location.methods.empty())
        return (true);
    for (size_t i = 0; i < location.methods.size(); ++i)
    {
        if (location.methods[i] == method)
            return (true);
    }
    return (false);
}