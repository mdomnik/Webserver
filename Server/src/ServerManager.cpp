/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:49:44 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/28 16:34:02 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerManager.hpp"

// ==== Constructor and Destructor ====
ServerManager::ServerManager(const std::vector<ServerConfig> &configs) : _epollFD(-1)
{
	InitServers(configs);
	if(_servers.empty())
		throw std::runtime_error("Server Manager | No servers found");
		
	InitEpoll();
	AddListenSocketsToEpoll();

	std::cout << "Server Manager | ServerManager initialized with " << _servers.size() << " servers. Accepting connections..." << std::endl;
}

ServerManager::~ServerManager()
{
	ShutdownServers();
}


// ==== Private Initialization Methods ====

// Initializes servers based on config file
void ServerManager::InitServers(const std::vector<ServerConfig>& configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
	{
		Server server(configs[i]);
		server.Start();
		_servers.push_back(server);
	}
}

// Initializes the epoll instance
void ServerManager::InitEpoll()
{
	_epollFD = epoll_create(128);
	if (_epollFD == -1)
		throw std::runtime_error("Server Manager | failed to initialize epoll");
}

// ==== Private Event Handling Methods ====

// Sets a file descriptor to non-blocking mode
void ServerManager::SetNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < -0) // set fd to non-blocking
	{
		close (fd);
		throw std::runtime_error("Server Manager | failed to set the fd to non-blocking");
	}
}

// Adds all server listening sockets to the epoll instance
void ServerManager::AddListenSocketsToEpoll()
{
	// Add each server's listening socket to epoll
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		int fd = _servers[i].GetSocketFD();
		struct epoll_event epollEvent;
		std::memset(&epollEvent, 0, sizeof(epollEvent));

		// Set the events to monitor
		epollEvent.events = EPOLLIN; // monitor for read events
		epollEvent.data.fd = fd;

		// Add the server socket to the epoll instance
		if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, fd, &epollEvent) == -1)
		{
			close(fd);
			close(_epollFD);
			throw std::runtime_error("Server Manager | failed to add server socket to epoll");
		}
	}
}

// Handles new client connections for a given server
void ServerManager::HandleNewConnections(int listening, Server &server)
{
	// Accept all pending connections
	while (true)
	{
		// Accept a new client connection
		struct sockaddr_in clientAddress;
		socklen_t lengthOfAddress = sizeof(clientAddress);
		int client_fd = accept(listening, (struct sockaddr*)&clientAddress, &lengthOfAddress);
		
		// Check for errors
		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // no more pending connections
				break;
			else // other error
			{
				std::cerr << "Server Manager | problem with accept(): " << std::strerror(errno) << std::endl;
				break;
			}
		}

		// Set client socket to non-blocking
		SetNonBlocking(client_fd);
	
		// Map client fd to its appropriate server
		_clientToServer[client_fd] = &server;

		// Add client socket to epoll monitoring
		struct epoll_event event;
		std::memset(&event, 0, sizeof(event));
		event.events = EPOLLIN;
		event.data.fd = client_fd;

		// Add to epoll
		if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &event) == -1)
		{
			close(client_fd);
			_clientToServer.erase(client_fd);
			std::cerr << "Server Manager | failed to add client socket to epoll: " << std::strerror(errno) << std::endl;
			continue;
		}

		// Log the accepted client
		std::cout << "Server Manager | Accepted new client with fd: " << client_fd << std::endl;
	
	}
}

// Handles activity on a client socket
void ServerManager::HandleClientActivity(int client_fd)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	// Read data from client
	ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) // Error or connection closed
	{
		CloseClient(client_fd);
		return;
	}

	std::cout << "Server Manager | Received data from client fd " << client_fd << ": " << buffer << std::endl;
	
	// parsing and response logic would go here
	CloseClient(client_fd); // Close client after handling for this example
}

// Closes a client connection and cleans up
void ServerManager::CloseClient(int client_fd)
{
	// Remove from epoll monitoring
	struct epoll_event event;
	std::memset(&event, 0, sizeof(event));
	epoll_ctl(_epollFD, EPOLL_CTL_DEL, client_fd, &event);

	// Close the client socket
	close(client_fd);

	// Remove from client-server mapping
	if (_clientToServer.find(client_fd) != _clientToServer.end())
		_clientToServer.erase(client_fd);

	std::cout << "Server Manager | Closed connection for client fd: " << client_fd << std::endl;
}

// ==== Public Server Operations ====

// the main event loop handling all server events
void ServerManager::RunLoop()
{
	struct epoll_event events[MAX_EVENTS];

	while (true)
	{
		// Wait for events
		int numberOfEvents = epoll_wait(_epollFD, events, MAX_EVENTS, -1);
		if (numberOfEvents < 0) // if error
		{
			if (errno == EINTR) // interrupted by signal restart loop
				continue;
			throw std::runtime_error("Server Manager | epoll_wait failed");
		}

		for (int i = 0; i < numberOfEvents; ++i) // for each event
		{
			int fileDescriptor = events[i].data.fd;
			
			bool isListening = false;
			for (size_t j = 0; j < _servers.size(); ++j) // check if it's a listening socket
			{
				if (fileDescriptor == _servers[j].GetSocketFD()) // if listening socket then handle new connections
				{
					isListening = true;
					HandleNewConnections(fileDescriptor, _servers[j]);
					break;
				}
			}
			if (isListening)
				continue;
			HandleClientActivity(fileDescriptor); // handle client activity
		}
	}
	ShutdownServers(); //interrupted, shutdown servers
}

void ServerManager::ShutdownServers()
{
	std::map<int, Server*>::iterator it = _clientToServer.begin();
	while (it != _clientToServer.end())
	{
		int client_fd = it->first;
		close(client_fd);
		++it;
	}
	_clientToServer.clear();

	
}