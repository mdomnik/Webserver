/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:28:20 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/28 14:01:52 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"
#include <iostream>

int main() {
    try {
        ConfigParser parser("Config/test/test.conf");
        std::vector<ServerConfig> servers = parser.parse();

        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Server " << i+1 << " (" << servers[i].host
                      << ":" << servers[i].port << ")\n";
            for (size_t j = 0; j < servers[i].locations.size(); ++j) {
                const LocationConfig &loc = servers[i].locations[j];
                std::cout << "  Location " << loc.path
                          << " root=" << loc.root
                          << " autoindex=" << (loc.autoIndex ? "on" : "off") << "\n";
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}