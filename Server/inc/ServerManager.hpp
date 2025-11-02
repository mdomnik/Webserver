/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:08:58 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 21:12:54 by fjoestin         ###   ########.fr       */
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

#include "../../Config/inc/ServerConfig.hpp"
#include "../../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../../HTTP/HTTPResponse/inc/HTTPResponse.hpp"
#include "Server.hpp"

#define MAX_EVENTS 64 // Maximum number of poll events

class ServerManager
{
	private:
		int					_epollFD; // epoll file descriptor
		std::vector<Server>	_servers; // list of managed servers
		std::map<int, Server*> _clientToServer; // map client fds to their servers
		std::map<int, HTTPRequest> _clientParsers; // map client fds to their HTTP request parsers
		// std::map<int, time_t> _clientLastActivity; // map to track the last activity of each connected client
		
		// initialization methods
		void InitServers(const std::vector<ServerConfig>& serverConfigs);
		void InitEpoll();

		// event handling methods
		void SetNonBlocking(int fd);
		void AddListenSocketsToEpoll();
		void HandleNewConnections(int listening, Server &server);
		void HandleClientActivity(int clientFD);
		void CloseClient(int clientFD);
		// void CheckTimeouts();

	public:
		// Constructor and Destructor
		ServerManager(const std::vector<ServerConfig> &configs);
		~ServerManager();

		// Server operations
		void RunLoop(); // Main event loop
		void RunLoopStep();
		void ShutdownServers(); // Shutdown all servers
};

#endif