/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:13:26 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 12:30:36 by mdomnik          ###   ########.fr       */
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
		int		_socketFD;
		struct sockaddr_in	_serverAddr;
		std::vector<int>	_clientSockets;
		ServerConfig		_config;

		// Server setup methods
		void CreateSocket();
		void BindandListen();

	public:
		// Constructor and Destructor
		Server(const ServerConfig& config);
		~Server();

		// Server operations
		void Start();
		void Stop();
		
		// Getters
		int GetSocketFD() const;
		const ServerConfig& GetServerConfig() const;

		// Client handling
		int acceptClient();
};

#endif