/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:13:53 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/28 16:15:33 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

// ==== Contructor and Destructor ====

// Constructor
Server::Server(const ServerConfig& config) : _socketFD(-1), _config(config)
{
	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
	_clientSockets.reserve(100); // reserve space for 100 clients
}

// Destructor
Server::~Server()
{
	Stop();
}

// ==== Private Server Operations ====

// Creates the server socket
void Server::CreateSocket()
{
	_socketFD = socket(AF_INET, SOCK_STREAM, 0); // get TCP socket
	if (_socketFD < 0)
		throw std::runtime_error("Server | Failed to create socket");
	
	int optionFlags = 1;
	if (setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &optionFlags, sizeof(optionFlags)) < 0) // set socket options that allow reuse of addr/port
	{
		close(_socketFD);
		throw std::runtime_error("Server | Failed to set socket options");
	}

	int flags = fcntl(_socketFD, F_GETFL, 0);
	if (flags < 0 || fcntl(_socketFD, F_SETFL, flags | O_NONBLOCK) < 0) // set socket to non-blocking
	{
		close(_socketFD);
		throw std::runtime_error("Server | Failed to set socket to non-blocking");
	}
}

// Binds and listens on the server socket
void Server::BindandListen()
{
	// Bind the socket to the specified host and port
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(_config.port);
	_serverAddr.sin_addr.s_addr = inet_addr(_config.host.c_str());

	if (_serverAddr.sin_addr.s_addr == INADDR_NONE) // if invalid IP address
	{
		close(_socketFD);
		throw std::runtime_error("Server | Invalid IP address: " + _config.host);
	}

	if (bind(_socketFD, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0) // bind socket
	{
		close(_socketFD);
		throw std::runtime_error("Server | Failed to bind socket to address");
	}

	if (listen(_socketFD, SOMAXCONN) < 0) // start listening on socket
	{
		close(_socketFD);
		throw std::runtime_error("Server | Failed to listen on socket");
	}
}

// ==== Public Server Operations ====

// Starts the server and begins listening for connections
void Server::Start()
{
	CreateSocket();
	BindandListen();
	std::cout << "Server | Server started! Listening on " << _config.host << ":" << _config.port << " !" << std::endl;
}

// Stops the server and closes all connections
void Server::Stop()
{
	for (size_t i = 0; i < _clientSockets.size(); ++i)
		close(_clientSockets[i]);
	
	_clientSockets.clear();

	if (_socketFD != -1)
	{
		close(_socketFD);
		_socketFD = -1;
		std::cout << "Server | Server stopped running; All Clients disconnected." << std::endl;
	}
}

// ==== Getters ====

int Server::GetSocketFD() const
{
	return (_socketFD);
}

const ServerConfig& Server::GetServerConfig() const
{
	return (_config);
}

// ==== Client Handling ====

// Accepts a new client connection
int Server::acceptClient()
{
	// Accept a new client connection
	struct sockaddr_in address;
	socklen_t len = sizeof(address);
	int fd = accept(_socketFD, (struct sockaddr*)&address, &len);

	if (fd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN) // no pending connections (non-blocking)
			std::cerr << "Server | Error accepting client: " << std::strerror(errno) << std::endl;
		return (-1);
	}
	
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) // set client socket to non-blocking
	{
		close(fd);
		std::cerr << "Server | Failed to set client socket to non-blocking" << std::endl;
		return (-1);
	}

	_clientSockets.push_back(fd); // client socket added to the tracked list
	
	std::cout << "Server | Accepted new client with file descriptor (" << fd << ")" << std::endl;
	
	return (fd);
}



