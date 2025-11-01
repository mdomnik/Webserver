/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:49:44 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 13:59:47 by fjoestin         ###   ########.fr       */
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
	_servers.reserve(configs.size());
	for (size_t i = 0; i < configs.size(); ++i)
	{
		_servers.push_back(Server(configs[i]));
		_servers.back().Start();
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
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) // set fd to non-blocking
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
		const std::vector<int>& socketFDs = _servers[i].GetSocketFDs();
		for (size_t j = 0; j < socketFDs.size(); ++j)
		{
			struct  epoll_event epollEvent;
			std::memset(&epollEvent, 0, sizeof(epollEvent));
			epollEvent.events = EPOLLIN | EPOLLET; // Edge-triggered for listening sockets
			epollEvent.data.fd = socketFDs[j];
			if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, socketFDs[j], &epollEvent) == -1)
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
		int client_fd = server.acceptClient(listening);
		
		// Check for errors
		if (client_fd < 0)
			break;

		// Map client fd to its appropriate server
		_clientToServer[client_fd] = &server;
		_clientParsers[client_fd] = HTTPRequest();
		_clientLastActivity[client_fd] = std::time(NULL);

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
			_clientParsers.erase(client_fd);
			_clientLastActivity.erase(client_fd);
			continue;
		}

		std::cout << "Server Manager | Accepted new client with fd: " << client_fd << std::endl;
	}
}

// Handles activity on a client socket
void ServerManager::HandleClientActivity(int client_fd)
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	// Read data from client
	ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) // Error or connection closed
	{
		CloseClient(client_fd);
		return;
	}
	
	std::string chunk(buffer, bytesRead);
	// Parse the HTTP request chunk
	HTTPRequest& parser = _clientParsers[client_fd];
	ParseStatus status = parser.ParseRequestChunk(chunk);
	_clientLastActivity[client_fd] = std::time(NULL); // COME BACK!!!!!!!!!!!!

	if (status == Incomplete)
	{
		return; // wait for more data
	}
	if (status != Success || !parser.IsComplete()) // If there is any error found in parsing
	{
		// notify the server admin and close the connection
		std::cerr << "Server Manager | Wrong Request from Client " << client_fd << ": " << parser.GetErrorMessage() << std::endl;
		HTTPResponse error;
		error.SetResponseToError(400, "HTTP/1.1", "Bad Request");
		std::string badresponse = error.ResponseToString();
		send(client_fd, badresponse.c_str(), badresponse.size(), 0);
		CloseClient(client_fd);
		return;
	}

	const ServerConfig &config = _clientToServer[client_fd]->GetServerConfig();
	HTTPResponse response;
	bool keepAlive = parser.IsKeepAlive();
	response.SetHeader("Connection", keepAlive ? "keep-alive" : "close");
	std::string httpResponse = response.GenerateResponse(parser, config);
	// Send the response back to the client
	send(client_fd, httpResponse.c_str(), httpResponse.size(), 0);

	if (keepAlive)
	{
		parser.ResetRequest();
		_clientLastActivity[client_fd] = std::time(NULL);
		std::cout << "Server Manager | Keep-Alive active for client fd: " << client_fd << std::endl;
	}
	else
	{
		CloseClient(client_fd);
	}
}

// Closes a client connection and cleans up
void ServerManager::CloseClient(int client_fd)
{
	epoll_ctl(_epollFD, EPOLL_CTL_DEL, client_fd, 0);
	close(client_fd);

	// Remove from client-server mapping
	if (_clientToServer.find(client_fd) != _clientToServer.end())
		_clientToServer.erase(client_fd);
	if (_clientParsers.find(client_fd) != _clientParsers.end())
		_clientParsers.erase(client_fd);
	if (_clientLastActivity.find(client_fd) != _clientLastActivity.end())
		_clientLastActivity.erase(client_fd);


	std::cout << "Server Manager | Closed connection for client fd: " << client_fd << std::endl;
}

// ==== Public Server Operations ====

// the main event loop handling all server events
void ServerManager::RunLoop()
{
	struct epoll_event events[MAX_EVENTS];
	const int EPOLL_TIMEOUT_MS = 100;
	while (true)
	{
		// Wait for events
		int numberOfEvents = epoll_wait(_epollFD, events, MAX_EVENTS, EPOLL_TIMEOUT_MS); // add function that handles timeouts later
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
				const std::vector<int>& socketFDs = _servers[j].GetSocketFDs();
				for (size_t k = 0; k < socketFDs.size(); ++k)
				{
					if (fileDescriptor == socketFDs[k])
					{
						isListening = true;
						HandleNewConnections(socketFDs[k], _servers[j]); // handle new connections
						break;
					}
				}
				if (isListening)
					break;
			}
			if (!isListening)
				HandleClientActivity(fileDescriptor); // handle client activity
		}
		CheckTimeouts();
	}
	ShutdownServers(); //interrupted, shutdown servers
}

// Shuts down all servers and cleans up resources
void ServerManager::ShutdownServers()
{
	std::map<int, Server*>::iterator it = _clientToServer.begin();
	while (it != _clientToServer.end()) // close all client connections
	{
		int client_fd = it->first;
		close(client_fd);
		++it;
	}
	_clientToServer.clear();

	if (_epollFD != -1) // close epoll fd
	{
		close(_epollFD);
		_epollFD = -1;
	}

	for (size_t i = 0; i < _servers.size(); ++i) // stop all servers
	{
		_servers[i].Stop();
	}

	std::cout << "Server Manager | All servers shut down" << std::endl;
}

void ServerManager::CheckTimeouts()
{
    time_t now = std::time(NULL);
    std::vector<int> toClose;

    for (std::map<int, time_t>::iterator it = _clientLastActivity.begin(); it != _clientLastActivity.end(); ++it)
    {
        if (now - it->second > 10) // 10 seconds timeout
            toClose.push_back(it->first);
    }

    for (size_t i = 0; i < toClose.size(); ++i)
        CloseClient(toClose[i]);
}
