/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 12:58:50 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 13:07:21 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <cstddef>

#define MAX_BYTES 8192
#define MAX_SECTION_BYTES 16384
#define MAX_HEADER_COUNT 200
#define DEFAULT_MAX_BODY_SIZE 1000000 // 1MB

enum ParseStatus
{
	Success,
	Incomplete,
	BadRequest,
	NotImplemented,
	VersionNotSupported
};

enum ParseState
{
	RequestLineState,
	HeadersState,
	BodyState,
	DoneState,
	ErrorState
};

class HTTPRequest
{
	private:
		std::string
};

#endif