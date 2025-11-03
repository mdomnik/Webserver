/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 12:58:50 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/03 08:05:23 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cstddef>
#include <sstream>
#include <algorithm>
#include <cctype>
#include  <iostream>

#define CRLF "\r\n"
#define DOUBLECRLF "\r\n\r\n"

#define MAX_BYTES 1000000000
#define MAX_SECTION_BYTES 16384
#define MAX_HEADER_COUNT 200
#define DEFAULT_MAX_BODY_SIZE 1000000 // 1MB

enum ParseStatus
{
	Success,
	Incomplete,
	BadRequest,
	NotImplemented,
	VersionNotSupported,
	PayloadExceeded
};

enum ParseState
{
	RequestLineState,
	HeadersState,
	BodyState,
	ChunkedBodyState,
	DoneState,
	ErrorState
};

class HTTPRequest
{
	private:
		// Data Parsed from request
		std::string							_method;
		std::string							_path;
		std::string							_httpVersion;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		
		ParseState						_state;
		std::string						_buffer;
		std::string						_errorMessage;
		size_t							_maxBodySize;
	
	public:
		// Constructor
		HTTPRequest();

		// Parsing Methods
		ParseStatus ParseRequestChunk(const std::string &chunk);
		void ResetRequest();
		
		// Status Checkers
		bool IsComplete() const;
		bool HasError() const;
		const std::string& GetErrorMessage() const;

		// Getters
		const std::string& GetMethod() const;
		const std::string& GetPath() const;
		const std::string& GetHTTPVersion() const;
		const std::map<std::string, std::string>& GetHeaders() const;
		const std::string& GetBody() const;
		std::string GetHeadersString() const;
		std::string GetHeader(const std::string& key) const;
		bool HasHeader(const std::string& key) const;
		bool IsKeepAlive() const;

		void setMaxBodySize(size_t size);

		// Parsing stages
		ParseStatus ParseRequestLine();
		ParseStatus ParseHeaders();
		ParseStatus ParseBody();
		ParseStatus ParseChunkedBody();

		// content length validation
		ParseStatus ValidateContentLength(size_t &contentLength) const;
};

#endif