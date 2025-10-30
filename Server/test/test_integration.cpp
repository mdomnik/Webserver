/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_integration.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:22:47 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 14:01:34 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerManager.hpp"
#include "../../Config/inc/ConfigParser.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>

// === Simple client helper ===
int send_request(const char *ip, int port, const std::string &request)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "Client | Failed to create socket\n";
		return (-1);
	}

	struct sockaddr_in servaddr;
	std::memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);

	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		std::cerr << "Client | Connection failed to " << ip << ":" << port << "\n";
		close(sock);
		return (-1);
	}

	send(sock, request.c_str(), request.size(), 0);

	char buffer[2048];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

	if (bytes > 0)
		std::cout << "Client | Received Response:\n" << buffer << "\n";
	else
		std::cout << "Client | No response or connection closed.\n";

	close(sock);
	return (0);
}

int main(void)
{
	std::cout << "🧩 Integration Test: Config + Server + HTTPRequest\n";

	try
	{
		ConfigParser parser("../Config/test/test.conf");
		std::vector<ServerConfig> configs = parser.parse();
		std::cout << "[+] Loaded " << configs.size() << " server configs.\n";

		pid_t pid = fork();
		if (pid == 0)
		{
			// === Child process: Run server manager ===
			try
			{
				ServerManager manager(configs);
				manager.RunLoop();
			}
			catch (const std::exception &e)
			{
				std::cerr << "Server Manager Error: " << e.what() << std::endl;
			}
			exit(0);
		}
		else
		{
			// === Parent process: Give server a moment to start ===
			sleep(1);

			// Send requests to each port defined in the config
			std::cout << "Client | Sending test requests...\n";

			// GET request
			std::string req1 =
				"GET /index.html HTTP/1.1\r\n"
				"Host: localhost\r\n"
				"User-Agent: Integration-Test\r\n"
				"\r\n";
			send_request("127.0.0.1", 8080, req1);

			// POST request with small body
			std::string req2 =
				"POST /upload HTTP/1.1\r\n"
				"Host: 127.0.0.1\r\n"
				"Content-Length: 11\r\n"
				"\r\n"
				"Hello World";
			send_request("127.0.0.1", 8081, req2);

			// Bad request (to test 400 handling)
			std::string req3 = "BADREQUEST\r\n";
			send_request("127.0.0.1", 8080, req3);

			// Kill the server cleanly
			sleep(1);
			kill(pid, SIGINT);
			waitpid(pid, NULL, 0);

			std::cout << "✅ Integration test complete.\n";
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}