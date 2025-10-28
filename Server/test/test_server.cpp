/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:24:59 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/28 15:25:01 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Config/inc/ConfigParser.hpp"
#include "../inc/Server.hpp"
#include <iostream>

int main() {
	try {
		ConfigParser parser("../Config/test/test.conf");
		std::vector<ServerConfig> servers = parser.parse();

		Server s(servers[0]);
		s.Start();

		std::cout << "Press Enter to accept clients (CTRL+C to quit)" << std::endl;
		std::cin.get();

		while (true)
			s.acceptClient();

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
