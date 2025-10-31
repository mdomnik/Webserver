/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:31:18 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 20:07:21 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerConfig.hpp"

// ==== Constructors ====

// Default constructor for location blocks
LocationConfig::LocationConfig() : autoIndex(false), uploadEnable(false)
{
    path = "/";
    root = "";
    index = "index.html";
    uploadStore = "/tmp";
}

// Default constructor for server blocks
ServerConfig::ServerConfig() : serverName("Default") ,clientMaxBodySize(10485760)
{
	errorPages[404] = "/errors/404.html";
}