/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:21:23 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/29 14:43:54 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPRequest.hpp"

// Forward declarations for helper functions
static std::string TrimHTTPLine(const std::string& str);
static std::vector<std::string> HTTPSplitTokens(const std::string line);
static int HttpStringToSizet(const std::string &s, size_t &out);
static int HTTPValidateMethod(const std::string &method);
static int HTTPValidatePath(const std::string &path);
static int HTTPValidateVersion(const std::string &version);


// ==== Constructor ====
HTTPRequest::HTTPRequest() : _method(), _path(), _httpVersion(), _headers(), _body(), _state(RequestLineState), _buffer(), _errorMessage(), _maxBodySize(DEFAULT_MAX_BODY_SIZE) {}


// Preparing the class for a new erquest
void HTTPRequest::ResetRequest()
{
	_method.clear();
	_path.clear();
	_httpVersion.clear();
	_headers.clear();
	_body.clear();
	_buffer.clear();
	_errorMessage.clear();
	_state = RequestLineState;
}


// ==== State Checkers ====
bool HTTPRequest::IsComplete() const
{
	if (_state == DoneState)
		return (true);
	else
		return (false);
}

bool HTTPRequest::HasError() const
{
	if (_state == ErrorState)
		return (true);
	else
		return (false);
}

const std::string& HTTPRequest::GetErrorMessage() const { return (_errorMessage); }

// ==== Getters ====
const std::string& HTTPRequest::GetMethod() const { return (_method); }

const std::string& HTTPRequest::GetPath() const { return (_path); }

const std::string& HTTPRequest::GetHTTPVersion() const { return (_httpVersion); }

const std::map<std::string, std::string>& HTTPRequest::GetHeaders() const { return (_headers); }

const std::string& HTTPRequest::GetBody() const { return (_body); }


// ==== Main Parsing Loop ====

ParseStatus HTTPRequest::ParseRequestChunk(const std::string &chunk)
{
	// Append new chunk to buffer
	if (!chunk.empty())
		_buffer.append(chunk);
	
	// Main parsing loop
	while (true)
	{
		if (_state == RequestLineState) //checks which state we are in
		{
			ParseStatus status = ParseRequestLine(); //parses the current stage
			if (status != Success)
				return (status);
			_state = HeadersState;
		}
		if (_state == HeadersState)
		{
			ParseStatus status = ParseHeaders();
			if (status != Success)
				return (status);
		}
		if (_state == BodyState)
		{
			ParseStatus status = ParseBody();
			if (status != Success)
				return (status);
			_state = DoneState;
			return (Success);
		}
		if (_state == DoneState)
		{
			return (Success);
		}
		if (_state == ErrorState)
		{
			return (BadRequest);
		}

		return (Incomplete);
	}
}


// ==== Parsing Stages ====

// Parses the Request Line
ParseStatus HTTPRequest::ParseRequestLine()
{
	//finds end of line
	size_t EOL = _buffer.find(CRLF);
	if (EOL == std::string::npos)
	{
		if (_buffer.size() > MAX_BYTES) //if request line too long
		{
			_errorMessage = "Request Line too long";
			_state = ErrorState;
			return (BadRequest);
		}
		return (Incomplete);
	}

	std::string str = _buffer.substr(0, EOL);
	_buffer.erase(0, EOL + 2); // remove line and end of line from buffer
	
	//splits the line into tokens of method, path and version
	std::vector<std::string> components = HTTPSplitTokens(str);
	if (components.size() != 3) //if not exactly 3 parts
	{
		_errorMessage = "Malformed Request Line";
		_state = ErrorState;
		return (BadRequest);
	}

	if (!HTTPValidateMethod(components[0])) //validates method
	{
		_errorMessage = "Invalid HTTP Method";
		_state = ErrorState;
		return (NotImplemented);
	}
	if (!HTTPValidatePath(components[1])) //validates path
	{
		_errorMessage = "Invalid HTTP Path";
		_state = ErrorState;
		return (BadRequest);
	}
	if (!HTTPValidateVersion(components[2])) //validates version
	{
		_errorMessage = "Invalid HTTP Version";
		_state = ErrorState;
		return (VersionNotSupported);
	}
	
	//if succeeded, assigns values
	_method = components[0];
	_path = components[1];
	_httpVersion = components[2];

	return (Success);
}

// Parses the Headers
ParseStatus HTTPRequest::ParseHeaders()
{
	//find end of headers
	size_t headersEnd = _buffer.find(DOUBLECRLF);
	if (headersEnd == std::string::npos) //if there is no end
	{
		if (_buffer.size() > MAX_SECTION_BYTES) //if headers too long
		{
			_errorMessage = "Headers too long";
			_state = ErrorState;
			return (BadRequest);
		}
		return (Incomplete);
	}

	std::string str = _buffer.substr(0, headersEnd);
	_buffer.erase(0, headersEnd + 4); // remove headers and double CRLF
	
	
	_headers.clear();
	size_t iter = 0;
	int count = 0;
	while (iter < str.size()) //parses each header line into key value pairs
	{
		size_t end = str.find(CRLF, iter); //find end of line
		if (end == std::string::npos)
			end = str.size();
		
		std::string step = str.substr(iter, end - iter); //isolate line
		if (end < str.size())
			iter = end + 2;
		else
			iter = end;	
		
		if (step.empty()) //if empty line,
			break;
		if (count++ >= MAX_HEADER_COUNT) //if too many headers
		{
			_errorMessage = "Too many headers";
			_state = ErrorState;
			return (BadRequest);
		}

		// find the delimiter symbol
		size_t symbol = step.find(':');
		if (symbol == std::string::npos) //if no colon found
		{
			_errorMessage = "Malformed Header Line";
			_state = ErrorState;
			return (BadRequest);
		}

		//isolate header key value pair
		std::string key = TrimHTTPLine(step.substr(0, symbol));
		std::string value = TrimHTTPLine(step.substr(symbol + 1));

		if (key.empty() || value.empty()) //if empty key or value
		{
			_errorMessage = "Malformed Header Line";
			_state = ErrorState;
			return (BadRequest);
		}

		for (size_t i = 0; i < key.size(); ++i) //convert key to lowercase
			key[i] = std::tolower(static_cast<unsigned char>(key[i]));
		
		if (_headers.find(key) != _headers.end()) //if duplicate header
		{
			_errorMessage = "Duplicate Header: " + key;
			_state = ErrorState;
			return (BadRequest);
		}

		_headers[key] = value;
	}

	if (_httpVersion == "HTTP/1.1") // HTTP/1.1 requires Host header
	{
		// Find the Host header
		std::map<std::string,std::string>::const_iterator it = _headers.find("host");
		if (it == _headers.end() || TrimHTTPLine(it->second).empty())
		{
			_errorMessage = "Missing Host header in HTTP/1.1 request";
			_state = ErrorState;
			return (BadRequest);
		}
	}

	// check if there is content length header and validate it
	size_t ContentLength = 0;
	ParseStatus lengthStatus = validateContentLength(ContentLength);
	if (lengthStatus != Success)
	{
		_state = ErrorState;
		return (lengthStatus);
	}
	if (ContentLength > 0) //if there is content
		_state = BodyState;
	else
		_state = DoneState;
	
	return (Success);
}