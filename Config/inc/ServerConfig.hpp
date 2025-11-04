/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:14:38 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/04 15:09:58 by nmandakh         ###   ########.fr       */
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
    std::vector<std::string> methods; //list of accepted HTTP methods GET, POST, DELETE
    
	std::string path; //refers to the configuration path
    std::map<int, std::string> redirection; // HTTP redirect rule (return 301 http://example.com/)
    std::string root; //directory where the requested files are located for that route
	std::string alias; // alternative directory mapping for the location
	std::string fullpath;
	
    bool autoIndex; //enable or disable automatic directory listing
    std::string index; //default file, front page directory

    bool uploadEnable; //bool whether a user can upload files
    std::string uploadStore; //directory path, where uploaded files are saved

    std::string cgiExtension; //end of file that trigger type of cgi execution
    std::string cgiPath; //path to cgi interpreter
	
	LocationConfig();
};

// Configuration of a server block
struct ServerConfig
{
    std::string serverName; //Default name of the server
    
	std::vector<std::pair<std::string, int> > listens; //ip:port pairs on which we can access the server
    size_t clientMaxBodySize; // Maximum amount of connections per socket
    size_t _keepAliveTimeout;
	size_t cgiTimeout;
	
	std::map<int, std::string> errorPages; // direction to error pages
    
	std::vector<LocationConfig> locations; //vector container storing all the locations from this server

	ServerConfig();
};

#endif