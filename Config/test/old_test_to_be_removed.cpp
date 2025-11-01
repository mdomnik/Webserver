/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   base_test.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:28:20 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 22:51:19 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfigParser.hpp"
#include <iostream>
#include <fstream>
#include <vector>

int	lineNum(std::string	testList) {
	std::string		str;
	std::ifstream	testListSize(testList.c_str());
	
	int i = 0;
	while(getline(testListSize, str)) {
		i++;
	}

	testListSize.close();
	return i;
}

int main() {
	std::string	listFile("./test/tests.txt");

	int	lines = lineNum(listFile);
	std::cout << "lines: " << lines << "\n";

	std::ifstream	testList(listFile.c_str());
	std::vector<std::string>	tests(lines);

	int k = 0;
	while(getline(testList, tests[k])) {
		std::cout << tests[k] << "\n";
		k++;
	}
	testList.close();

	std::string	ESCAPE("\033[0m");
	std::string	DARKGREEN("\033[38;5;35m");
	std::string	LIGHTGREEN("\033[38;5;120m");
	std::cout << LIGHTGREEN << "STARTING CONFIGURATION TESTS..." << ESCAPE <<"\n";
	for (int file = 0; file < lines; file++) {
		std::cout << DARKGREEN << "File: " << tests[file] << ESCAPE << std::endl;
		try {
			ConfigParser parser(tests[file].c_str());
	
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
			std::cerr << "\nError: " << e.what() << std::endl;
		}
		std::cout << "NEXT TEST\n\n";
	}
}