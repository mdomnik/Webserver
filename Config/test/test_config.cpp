/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:28:20 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 15:58:00 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"
#include <iostream>
#include <fstream>

	std::string	ESCAPE("\033[0m");
	std::string	DARKGREEN("\033[38;5;35m");
	std::string	LIGHTGREEN("\033[38;5;120m");
	std::string	RED("\033[38;5;196m");
	std::string YELLOW("\033[38;5;226m");

static void printServer(const ServerConfig& server, size_t index)
{
    std::cout << YELLOW << "\n==================== SERVER " << index + 1 << " ====================\n" << ESCAPE;
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

    std::cout << YELLOW << "=====================================================\n" << ESCAPE;
}

int	testFilesNumber(std::string testListFile) {
	std::ifstream	file(testListFile.c_str());
	std::string		line;
	int				count = 0;

	if (!file.is_open()) {
		std::cerr << "Could not open the test list file: " << testListFile << std::endl;
		return 0;
	}
	while (std::getline(file, line)) {
		if (!line.empty())
			count++;
	}
	file.close();
	return count;
}

int	explicitConfig(std::string configPath) {
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
		if (configPath.find("invalid") != std::string::npos) return 0;
        return 1;
    }
	return 0;
}

int	testFiles(int totalTests) {
	std::ifstream	file("./test/tests.txt");
	std::string		line;
	int				testNumber = 0;
	int				failedTests = 0;

	if (!file.is_open()) {
		std::cerr << "Could not open the test list file: ./test/tests.txt" << std::endl;
		return 1;
	}
	while (std::getline(file, line)) {
		if (line.empty())
			continue;
		testNumber++;
		std::cout << DARKGREEN << "\n=== Running Test " << testNumber << " of " << totalTests << ": " << line << " ===\n" << ESCAPE;
		int result = explicitConfig(line);
		if (result != 0) {
			failedTests++;
			std::cout << "\n" << RED << "=== Test " << testNumber << " FAILED ===\n" << ESCAPE;
		} else {
			std::cout << "\n" << LIGHTGREEN << "=== Test " << testNumber << " PASSED ===\n" << ESCAPE;
		}
	}
	file.close();
	std::cout << LIGHTGREEN << "\n=== Testing Complete: " << (totalTests - failedTests) << " Passed, " << failedTests << " Failed ===\n" << ESCAPE;

	return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>\n";
        return 1;
    } else if (argc == 2 && std::string (argv[1]) == "tests") {
		std::cout << LIGHTGREEN << "Running all tests from test list file...\n" << ESCAPE;
		// int	i = testFilesNumber("./test/tests.txt");
		testFiles(testFilesNumber("./test/tests.txt"));
	}

    std::string configPath = argv[1];

    return explicitConfig(configPath);
}