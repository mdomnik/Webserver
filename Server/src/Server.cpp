/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:13:53 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 20:48:15 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

// ==== Contructor and Destructor ====

// Constructor
Server::Server(const ServerConfig& config) : _config(config)
{
	_socketFDs.clear();
	_serverAddrs.clear();
	_clientSockets.clear();
}

// Destructor
Server::~Server()
{
	for (size_t i = 0; i < _socketFDs.size(); ++i)
		close(_socketFDs[i]);
	for (size_t i = 0; i < _clientSockets.size(); ++i)
		close(_clientSockets[i]);
}

// ==== Private Server Operations ====

// Creates the server socket
int Server::CreateSocket()
{
	int socketFD = socket(AF_INET, SOCK_STREAM, 0); // get TCP socket
	if (socketFD < 0)
		throw std::runtime_error("Server | Failed to create socket");
	
	int optionFlags = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &optionFlags, sizeof(optionFlags)) < 0) // set socket options that allow reuse of addr/port
	{
		close(socketFD);
		throw std::runtime_error("Server | Failed to set socket options");
	}

	int flags = fcntl(socketFD, F_GETFL, 0);
	if (flags < 0 || fcntl(socketFD, F_SETFL, flags | O_NONBLOCK) < 0) // set socket to non-blocking
	{
		close(socketFD);
		throw std::runtime_error("Server | Failed to set socket to non-blocking");
	}

	return (socketFD);
}

// ==== Public Server Operations ====

// Starts the server and begins listening for connections
void Server::Start()
{
	if (_config.listens.empty())
		throw std::runtime_error("Server | No listen addresses configured");
	for (size_t i = 0; i < _config.listens.size(); ++i)
	{
		std::string host = _config.listens[i].first;
		int port = _config.listens[i].second;

		struct sockaddr_in serverAddr;
		std::memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		
		if (host == "localhost")
			serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		else if (host == "any" || host == "0.0.0.0")
			serverAddr.sin_addr.s_addr = INADDR_ANY;
		else
		{
			if (inet_aton(host.c_str(), &serverAddr.sin_addr) == 0)
			{
				std::cerr << "Server | Invalid IP address: " << host << std::endl;
				continue;
			}
		}

		int socketFD = CreateSocket();

		// Try to bind and listen
		if (bind(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			std::cerr << "Server | Failed to bind" << std::endl;
			close(socketFD);
			continue;
		}
		if (listen(socketFD, SOMAXCONN) < 0)
		{
			std::cerr << "Server | Failed to listen" << std::endl;
			close(socketFD);
			continue;
		}

		_socketFDs.push_back(socketFD);
		_serverAddrs.push_back(serverAddr);

		std::cout << "Server | Listening on " << host << ":" << port << std::endl;
	}

	if (_socketFDs.empty())
		throw std::runtime_error("Server | No valid listen sockets available to start server");
}

// Stops the server and closes all connections
void Server::Stop()
{
	for (size_t i = 0; i < _clientSockets.size(); ++i)
		close(_clientSockets[i]);
	_clientSockets.clear();

	for (size_t i = 0; i < _socketFDs.size(); ++i)
		close(_socketFDs[i]);
	_socketFDs.clear();
	
	std::cout << "Server | Stopped and closed all connections." << std::endl;
}

// ==== Getters ====

const std::vector<int>& Server::GetSocketFDs() const
{
	return (_socketFDs);
}

const ServerConfig& Server::GetServerConfig() const
{
	return (_config);
}

// ==== Client Handling ====

// Accepts a new client connection
int Server::acceptClient(int socketFD)
{
	// Accept a new client connection
	struct sockaddr_in address;
	socklen_t len = sizeof(address);
	int fd = accept(socketFD, (struct sockaddr*)&address, &len);
	if (fd < 0)
		return (-1);

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(fd);
		return (-1);
	}
	_clientSockets.push_back(fd);

	std::cout << "Server | New client connected with fd: " << fd << std::endl;

	return (fd);
}



