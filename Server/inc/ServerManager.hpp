/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:08:58 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/04 15:58:55 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <ctime>
#include <csignal>
#include <set>

#include "../../Config/inc/ServerConfig.hpp"
#include "../../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../../HTTP/HTTPResponse/inc/HTTPResponse.hpp"
#include "Server.hpp"

#define MAX_EVENTS 64 // Maximum number of poll events

struct CGIState {
    time_t start_time;
    pid_t pid;
	time_t timeoutLimit;
};

class ServerManager
{
	private:
		int					_epollFD; // epoll file descriptor
		std::vector<Server>	_servers; // list of managed servers
		std::map<int, Server*> _clientToServer; // map client fds to their servers
		std::map<int, HTTPRequest> _clientParsers; // map client fds to their HTTP request parsers
		std::map<int, time_t> _clientLastActivity; // map to track the last activity of each connected client
		std::map<int, int> _cgiToClient;      // CGI fd → client fd
		std::map<int, std::string> _cgiBuffers; // CGI fd → partial output
		std::set<int> _cgiFDs; 
		std::map<int, CGIState> _cgiState;
		// initialization methods
		void InitServers(const std::vector<ServerConfig>& serverConfigs);
		void InitEpoll();

		// event handling methods
		void SetNonBlocking(int fd);
		void AddListenSocketsToEpoll();
		void HandleNewConnections(int listening, Server &server);
		void HandleClientActivity(int clientFD);
		void HandleCGIOutput(int cgi_fd, uint32_t events);
		void CloseClient(int clientFD);
		void CheckTimeouts();
		bool endsWith(const std::string &str, const std::string &suffix);

	public:
		// Constructor and Destructor
		ServerManager(const std::vector<ServerConfig> &configs);
		~ServerManager();

		bool IsCGIRequest(const HTTPRequest &req, const ServerConfig &config, LocationConfig &loc);
		// Server operations
		void RunLoop(); // Main event loop
		void ShutdownServers(); // Shutdown all servers
};

#endif