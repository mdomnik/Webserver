/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_http_request.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:10:58 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 13:16:24 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPRequest.hpp"
#include <iostream>

static void runTest(const std::string &raw)
{
    HTTPRequest req;
    ParseStatus status = req.ParseRequestChunk(raw);

    if (status == Incomplete)
        std::cout << "[!] Incomplete request" << std::endl;
    else if (status == BadRequest)
        std::cout << "[!] Bad request: " << req.GetErrorMessage() << std::endl;
    else if (status == NotImplemented)
        std::cout << "[!] Not implemented: " << req.GetErrorMessage() << std::endl;
    else if (status == VersionNotSupported)
        std::cout << "[!] Version not supported: " << req.GetErrorMessage() << std::endl;
    else if (status == Success)
        std::cout << "[+] Request parsed successfully!" << std::endl;

    if (req.IsComplete())
    {
        std::cout << "Method:  " << req.GetMethod() << std::endl;
        std::cout << "Path:    " << req.GetPath() << std::endl;
        std::cout << "Version: " << req.GetHTTPVersion() << std::endl;
        std::cout << "Headers:" << std::endl;

        std::map<std::string,std::string> headers = req.GetHeaders();
        for (std::map<std::string,std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
            std::cout << "  " << it->first << ": " << it->second << std::endl;

        std::cout << "Body:    [" << req.GetBody() << "]" << std::endl;
    }

    std::cout << "------------------------------------" << std::endl;
}

int main()
{
    // === Basic GET ===
    std::string req1 =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: test-client\r\n"
        "\r\n";

    runTest(req1);

    // === POST with body ===
    std::string req2 =
        "POST /upload HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";

    runTest(req2);

    // === Malformed ===
    std::string req3 =
        "BADREQUEST\r\n\r\n";

    runTest(req3);

    // === Chunked feed simulation ===
    HTTPRequest req4;
    std::string part1 = "GET /test HTTP/1.1\r\nHost: local";
    std::string part2 = "host\r\nUser-Agent: curl\r\n\r\n";

    std::cout << "[Partial feed #1]" << std::endl;
    ParseStatus s1 = req4.ParseRequestChunk(part1);
    std::cout << "Result: " << s1 << " (expected incomplete)" << std::endl;

    std::cout << "[Partial feed #2]" << std::endl;
    ParseStatus s2 = req4.ParseRequestChunk(part2);
    std::cout << "Result: " << s2 << " (expected success)" << std::endl;

    if (req4.IsComplete())
    {
        std::cout << "Parsed final request successfully:" << std::endl;
        std::cout << "Method: " << req4.GetMethod() << " Path: " << req4.GetPath() << std::endl;
    }

    return 0;
}
