/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 13:21:23 by mdomnik           #+#    #+#             */
/*   Updated: 2025/11/02 22:51:35 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPRequest.hpp"
#include "../inc/HTTPRequestUtils.hpp"

// ==== Constructor ====
HTTPRequest::HTTPRequest() : _method(), _path(), _httpVersion(), _headers(), _body(), _state(RequestLineState), _buffer(), _errorMessage(), _maxBodySize(DEFAULT_MAX_BODY_SIZE) {}

void HTTPRequest::setMaxBodySize(size_t size) { _maxBodySize = size; }

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

std::string HTTPRequest::GetHeader(const std::string& key) const
{
	std::string lower = key;
	for (size_t i = 0; i < lower.size(); ++i)
		lower[i] = std::tolower(static_cast<unsigned char>(lower[i]));

	std::map<std::string, std::string>::const_iterator it = _headers.find(lower);
	if (it != _headers.end())
		return it->second;
	return "";
}

bool HTTPRequest::HasHeader(const std::string& key) const
{
    std::string lower = key;
    for (size_t i = 0; i < lower.size(); ++i)
        lower[i] = std::tolower(static_cast<unsigned char>(lower[i]));

    return _headers.find(lower) != _headers.end();
}

bool HTTPRequest::IsKeepAlive() const
{
    // Default: HTTP/1.1 = keep-alive unless stated otherwise
    if (_httpVersion == "HTTP/1.1")
    {
        std::map<std::string, std::string>::const_iterator it = _headers.find("connection");
        if (it != _headers.end())
        {
            std::string val = it->second;
            for (size_t i = 0; i < val.size(); ++i)
                val[i] = std::tolower(static_cast<unsigned char>(val[i]));
            if (val == "close")
                return false;
        }
        return true; // implicit keep-alive
    }

    // For HTTP/1.0, keep-alive only if explicitly stated
    if (_httpVersion == "HTTP/1.0")
    {
        std::map<std::string, std::string>::const_iterator it = _headers.find("connection");
        if (it != _headers.end())
        {
            std::string val = it->second;
            for (size_t i = 0; i < val.size(); ++i)
                val[i] = std::tolower(static_cast<unsigned char>(val[i]));
            if (val == "keep-alive")
                return true;
        }
    }

    return false;
}



// ==== Main Parsing Loop ====

ParseStatus HTTPRequest::ParseRequestChunk(const std::string &chunk)
{
	// Append new chunk to buffer
	if (!chunk.empty())
		_buffer.append(chunk);
		// std::cout << "buffer: " << _buffer << std::endl;
	
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
		if (_state == ChunkedBodyState)
		{
			ParseStatus status = ParseChunkedBody();
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
		std::cout << "Count of requrst " << components.size() << std::endl;
		for (size_t i = 0; i < components.size(); ++i)
		{
			std::cout << "string: " << components[i] << std::endl; 
		}
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

	std::map<std::string, std::string>::const_iterator enc = _headers.find("transfer-encoding");
	if (enc != _headers.end())
	{
		std::cout << "ENTERED TRANSFER-ENCODING" << std::endl;
		std::string val = enc->second;
		for (size_t i = 0; i < val.size(); ++i)
			val[i] = std::tolower(static_cast<unsigned char>(val[i]));
		if (val == "chunked")
		{
			_state = ChunkedBodyState;
			return (Success);
		}
	}

	// check if there is content length header and validate it
	size_t ContentLength = 0;
	ParseStatus lengthStatus = ValidateContentLength(ContentLength);
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

// Parses the Body
ParseStatus HTTPRequest::ParseBody()
{
	// Determine expected content length
	size_t contentLength = 0;
	if (ValidateContentLength(contentLength) != Success)
	{
		_state = ErrorState;
		return (BadRequest);
	}

	if (_buffer.size() < contentLength) //if body not fully received
		return (Incomplete);
	std::cout << _maxBodySize << "MAXBODY" << std::endl;

	if (contentLength > _maxBodySize) //if body too large
	{
		_errorMessage = "Body size exceeds maximum allowed";
		_state = ErrorState;
		return (PayloadExceeded);
	}

	_body.assign(_buffer, 0, contentLength); //extract body
	_buffer.erase(0, contentLength); //remove body from buffer

	return (Success);
}

// Parses the Chunked Body
// ParseStatus HTTPRequest::ParseChunkedBody()
// {
// 	while (true)
// 	{
// 		// Find CRLF marking end of current chunk size
// 		size_t crlfPos = _buffer.find(CRLF);
// 		if (crlfPos == std::string::npos)
// 			return Incomplete;

// 		// Extract chunk size in hex
// 		std::string sizeLine = _buffer.substr(0, crlfPos);
// 		size_t chunkSize = 0;
// 		std::stringstream ss;
// 		ss << std::hex << sizeLine;
// 		ss >> chunkSize;

// 		// Remove the size line + CRLF
// 		_buffer.erase(0, crlfPos + 2);

// 		if (chunkSize == 0)
// 		{
// 			// Final chunk (ends with \r\n)
// 			size_t ending = _buffer.find(CRLF);
// 			if (ending != std::string::npos)
// 				_buffer.erase(0, ending + 2);
// 			_state = DoneState;
// 			return Success;
// 		}

// 		// Wait until we have full chunk data + trailing CRLF
// 		if (_buffer.size() < chunkSize + 2)
// 			return Incomplete;

// 		// Append chunk data to body
// 		_body.append(_buffer, 0, chunkSize);

// 		std::cout << _maxBodySize << "MAXBODY" << std::endl;
// 		if (_body.size() > _maxBodySize) // Check max body size
// 		{
// 			_errorMessage = "Body size exceeds maximum allowed";
// 			_state = ErrorState;
// 			return (PayloadExceeded);
// 		}

// 		// Remove chunk data + CRLF
// 		_buffer.erase(0, chunkSize + 2);
// 	}
// 	return (Success);
// }

ParseStatus HTTPRequest::ParseChunkedBody()
{
    // Try to decode as much as possible from _buffer.
    size_t pos = 0;

    while (true)
    {
        // Find CRLF marking end of chunk-size line
        size_t endOfSize = _buffer.find(CRLF, pos);
        if (endOfSize == std::string::npos)
            return Incomplete; // need more data

        // Extract size line
        std::string sizeLine = _buffer.substr(pos, endOfSize - pos);
        size_t semi = sizeLine.find(';');
        if (semi != std::string::npos)
            sizeLine = sizeLine.substr(0, semi);

        // Trim spaces
        while (!sizeLine.empty() && std::isspace(sizeLine[0]))
            sizeLine.erase(sizeLine.begin());
        while (!sizeLine.empty() && std::isspace(sizeLine[sizeLine.size() - 1]))
            sizeLine.erase(sizeLine.end() - 1);

        if (sizeLine.empty())
        {
            _errorMessage = "Invalid chunk size line";
            _state = ErrorState;
            return BadRequest;
        }

        // Convert from hex
        size_t chunkSize = 0;
        std::stringstream ss;
        ss << std::hex << sizeLine;
        ss >> chunkSize;

        // Move buffer past the size line + CRLF
        pos = endOfSize + 2;

        // Check for final chunk (size = 0)
        if (chunkSize == 0)
        {
            // Wait for final CRLF or optional trailers
            size_t trailerEnd = _buffer.find(DOUBLECRLF, pos);
            if (trailerEnd == std::string::npos)
            {
                // Some clients just send one CRLF
                size_t singleEnd = _buffer.find(CRLF, pos);
                if (singleEnd == std::string::npos)
                    return Incomplete; // wait for end
                _buffer.erase(0, singleEnd + 2);
            }
            else
                _buffer.erase(0, trailerEnd + 4);

            _state = DoneState;
            return Success;
        }

        // Make sure the whole chunk data + CRLF is in buffer
        if (_buffer.size() < pos + chunkSize + 2)
            return Incomplete;

        // Append this chunk to body
        _body.append(_buffer, pos, chunkSize);

        // if (_body.size() > _maxBodySize)
        // {
        //     _errorMessage = "Body size exceeds maximum allowed";
        //     _state = ErrorState;
        //     return PayloadExceeded;
        // }
		// std::cout << "current body size: " << _body.size() << std::endl;
        // Remove this chunk + CRLF from buffer
        _buffer.erase(0, pos + chunkSize + 2);

        // Reset pos for next loop
        pos = 0;
    }

    return Success;
}


// Validates the Content-Length header
ParseStatus HTTPRequest::ValidateContentLength(size_t &length) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find("content-length");
	if (it == _headers.end())
	{
		length = 0;
		return (Success);
	}

	size_t convert = 0;
	if (!HttpStringToSizet(it->second, convert))
		return (BadRequest);
	if (convert > _maxBodySize)
		return (BadRequest);

	length = convert;
	return (Success);
}