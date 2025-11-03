/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 14:12:00 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/03 17:18:01 by nmandakh         ###   ########.fr       */
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
