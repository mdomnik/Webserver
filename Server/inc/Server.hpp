/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:13:26 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 13:27:02 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

#include "../../Config/inc/ServerConfig.hpp"

class Server
{
	private:
		std::vector<int>				_socketFDs;
		std::vector<struct sockaddr_in>	_serverAddrs;
		std::vector<int>				_clientSockets;
		ServerConfig					_config;

		// Server setup methods
		int CreateSocket();
	public:
		// Constructor and Destructor
		Server(const ServerConfig& config);
		~Server();

		// Server operations
		void Start();
		void Stop();
		
		// Getters
		const std::vector<int>& GetSocketFDs() const;
		const ServerConfig& GetServerConfig() const;

		// Client handling
		int acceptClient(int socketFD);
};

#endif