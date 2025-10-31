/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 16:21:08 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/31 16:44:38 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Config/inc/ConfigParser.hpp"
#include "../Server/inc/ServerManager.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return (1);
	}

	try
	{
		ConfigParser parser(argv[1]);
		std::vector<ServerConfig> serverConfigs = parser.parse();

		ServerManager serverManager(serverConfigs);
		serverManager.RunLoop();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}