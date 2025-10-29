/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:24:59 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 12:15:58 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Config/inc/ConfigParser.hpp"
#include "../inc/ServerManager.hpp"
#include <iostream>

int main() {
    try {
        // Use your existing config file
        ConfigParser parser("../Config/test/test.conf");
        std::vector<ServerConfig> configs = parser.parse();

        // (Optional) call ValidateServer(configs[i]) if you have it.
        for (size_t i = 0; i < configs.size(); ++i) {
            // ValidateServer(configs[i]);
        }

        ServerManager mgr(configs);
        mgr.RunLoop(); // blocks here

    } catch (const std::exception &e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
    }
    return 0;
}
