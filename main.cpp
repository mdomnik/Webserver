/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 14:12:00 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 12:56:59 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/inc/ServerManager.hpp"
#include "Config/inc/ConfigParser.hpp"
#include <iostream>
#include <csignal>

// Global flag updated by the signal handler
volatile sig_atomic_t g_stopSignal = 0;

// Simple signal handler
void HandleSignal(int sig)
{
	if (sig == SIGINT)
	{
		std::cout << "\n🛑 Caught SIGINT — stopping server gracefully..." << std::endl;
		g_stopSignal = 1;
	}
}

int main(int argc, char **argv)
{
	try
	{
		// === Register signal handler ===
		signal(SIGINT, HandleSignal);
		signal(SIGTERM, HandleSignal);

		// === Parse config file ===
		std::string configPath;
		if (argc == 2)
			configPath = argv[1];
		else
			configPath = "ConfigFiles/default.conf";

		std::cout << "🧩 Starting Webserv with config: " << configPath << std::endl;

		ConfigParser parser(configPath);
		std::vector<ServerConfig> configs = parser.parse();

		if (configs.empty())
			throw std::runtime_error("No server configurations found!");

		std::cout << "[+] Loaded " << configs.size() << " server(s)." << std::endl;

		// === Create ServerManager ===
		ServerManager manager(configs);

		// === Main event loop ===
		std::cout << "🚀 Webserv is running. Press CTRL+C to stop.\n";
		while (!g_stopSignal)
		{
			manager.RunLoopStep(); // a single non-blocking iteration
		}

		// === Graceful shutdown ===
		std::cout << "🧹 Cleaning up..." << std::endl;
		manager.ShutdownServers();
		std::cout << "✅ Server stopped cleanly." << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
