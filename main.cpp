/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 14:12:00 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 22:11:13 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/inc/ServerManager.hpp"
#include "Config/inc/ConfigParser.hpp"
#include <iostream>
#include <csignal>




int main(int argc, char **argv)
{
	try
	{

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
		manager.RunLoop();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
