/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:14:38 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/28 11:35:08 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

// Configuration for a location block
struct LocationConfig
{
	std::string path; // path to location from root
	std::string root; // root specification

	bool autoIndex; // automatic directory listing for missing index
	std::string index; // path to index page

	std::vector<std::string> methods; // Methods allowed at locatioj

	std::string uploadStore; // directory for uploaded files
	std::string redirection; // redirects to different page
	std::string cgiExtention; // type od executable CGI extention
};

// Configuration of a server block
struct ServerConfig
{
	std::string host; // no address
	int port; // default port
	std::string serverName; // default name
	size_t clientMaxBodySize; // 1mb
	std::map<int, std::string> errorPages; // map of pages to err codes
	std::vector<LocationConfig> locations; // location blocks
};

#endif