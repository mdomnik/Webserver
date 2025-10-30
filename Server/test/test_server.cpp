/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:24:59 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 13:35:21 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Config/inc/ConfigParser.hpp"
#include "../inc/ServerManager.hpp"
#include <iostream>

int main()
{
    try
    {
        ConfigParser parser("../Config/test/test.conf");
        std::vector<ServerConfig> configs = parser.parse();

        std::cout << "Config parsed successfully: " << configs.size() << " server(s) loaded." << std::endl;

        ServerManager manager(configs);
        manager.RunLoop(); // Blocking event loop

    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
    }

    return 0;
}
