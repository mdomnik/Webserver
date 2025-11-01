/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:31:18 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/01 14:22:19 by fjoestin         ###   ########.fr       */
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
ServerConfig::ServerConfig() : serverName("Default") ,clientMaxBodySize(10485760), _keepAliveTimeout(10)
{
	errorPages[404] = "/errors/404.html";
}
