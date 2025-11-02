/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestUtils.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 12:27:48 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 14:44:09 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPRequest.hpp"

// Trims leading and trailing whitespace from a HTTP line
std::string TrimHTTPLine(const std::string &str)
{
	if (str.empty())
		return (str);

	size_t i = 0; //find the first non whitespace from left side
	while (i < str.size() && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'))
		++i;

	size_t j = str.size(); //find the first non whitespace from right side
	while (j > 0)
	{
		char character = str[j - 1];
		if (character == ' ' || character == '\t' || character == '\r' || character == '\n')
		{
			--i;
		}
		else
			break ; 
	}
	//form a new string from the trimmed indices
	return (str.substr(i, j - i));
}


// Iteratre through the HTTP line and attempt to split it into tokenized parts
std::vector<std::string> HTTPSplitTokens(const std::string &str)
{
	std::vector<std::string> tokens;
	std::string current;
	for (size_t i = 0; i < str.size(); ++i) //iterate through each character and see if there is whitespace
	{
		if (str[i] == ' ' || str[i] == '\t') // if whitespace, create token from previous
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
		}
		else // else, add character to current token
			current.push_back(str[i]);
	}
	if (!current.empty()) // form a token from remaining characters
		tokens.push_back(current);
	return (tokens);
}

//converts decimal string to size_t
int HttpStringToSizet(const std::string &str, size_t &value)
{
	if (str.empty())
		return (0);
		
	size_t total = 0;
	//Can only be made out of digits
	for (size_t i = 0; i < str.size(); ++i)
	{
		char character = str[i];
		if (!std::isdigit(static_cast<unsigned char>(str[i]))) // Check if digit
			return (0);
		
		int digit = character - 48;
		
		if (total > (total * 10 + digit)) // See if number does not overflow
			return (0);
		
		total = total * 10 + digit; // add the new digit
	}
	value = total;
	return (1);
}

// check if HTTP's method matches the predetermined set
int HTTPValidateMethod(const std::string &method)
{
	std::cout << "Validating HTTP Method: " << method << std::endl;
	if (method == "GET" || method == "POST" || method == "DELETE" || method == "PUT" || method == "HEAD" || method == "OPTIONS" || method == "PATCH")
		return (1);
	return (0);
}

//basic checks for path validity
int HTTPValidatePath(const std::string &path)
{
	if (path.empty())
		return (0);
	if (path[0] != '/')
		return (0);
	if (path.size() > MAX_BYTES)
		return (0);
	return (1);
}

// checks if the http vesion is 1.0 or 1.1
int HTTPValidateVersion(const std::string &version)
{
	if (!(version == "HTTP/1.1" || version == "HTTP/1.0"))
		return (0);
	return (1);
}