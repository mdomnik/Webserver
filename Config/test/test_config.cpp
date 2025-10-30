/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:28:20 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 20:34:34 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"
#include <iostream>

static void printServer(const ServerConfig& server, size_t index)
{
    std::cout << "\n==================== SERVER " << index + 1 << " ====================\n";
    std::cout << "Server Name: " << server.serverName << "\n";

    std::cout << "Listens:\n";
    for (size_t i = 0; i < server.listens.size(); ++i)
        std::cout << "  - " << server.listens[i].first << ":" << server.listens[i].second << "\n";

    std::cout << "Client Max Body Size: " << server.clientMaxBodySize << "\n";

    std::cout << "Error Pages:\n";
    for (std::map<int, std::string>::const_iterator it = server.errorPages.begin(); it != server.errorPages.end(); ++it)
        std::cout << "  " << it->first << " -> " << it->second << "\n";

    std::cout << "\n--- Locations (" << server.locations.size() << ") ---\n";
    for (size_t i = 0; i < server.locations.size(); ++i)
    {
        const LocationConfig& loc = server.locations[i];
        std::cout << "\n[" << i + 1 << "] Location Path: " << loc.path << "\n";

        std::cout << "  Methods: ";
        for (size_t j = 0; j < loc.methods.size(); ++j)
            std::cout << loc.methods[j] << (j + 1 < loc.methods.size() ? ", " : "");
        std::cout << "\n";

        std::cout << "  Root: " << loc.root << "\n";
        std::cout << "  Index: " << (loc.index.empty() ? "(none)" : loc.index) << "\n";
        std::cout << "  AutoIndex: " << (loc.autoIndex ? "on" : "off") << "\n";

        if (!loc.redirection.empty())
        {
            std::cout << "  Redirection:\n";
            for (std::map<int, std::string>::const_iterator it = loc.redirection.begin(); it != loc.redirection.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << "\n";
        }

        std::cout << "  Upload Enable: " << (loc.uploadEnable ? "on" : "off") << "\n";
        if (!loc.uploadStore.empty())
            std::cout << "  Upload Store: " << loc.uploadStore << "\n";

        if (!loc.cgiExtension.empty())
            std::cout << "  CGI: " << loc.cgiExtension << " via " << loc.cgiPath << "\n";
    }

    std::cout << "=====================================================\n";
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>\n";
        return 1;
    }

    std::string configPath = argv[1];

    try
    {
        ConfigParser parser(configPath);
        std::vector<ServerConfig> servers = parser.parse();

        std::cout << "✅ Parsing successful!\n";
        std::cout << "Total servers found: " << servers.size() << "\n";

        for (size_t i = 0; i < servers.size(); ++i)
            printServer(servers[i], i);
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}