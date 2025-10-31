#include "../HTTP/CGI/inc/CGIHandler.hpp"
#include "../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../Config/inc/ServerConfig.hpp"
#include <iostream>

int main() {
    std::cout << "🧩 CGI Execution Test (aligned with current API)\n";

    try {
        // --- Step 1: Create a fake HTTP request ---
        HTTPRequest req;
        std::string raw =
            "GET /cgi-bin/hello.py HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "User-Agent: TestCGI\r\n"
            "\r\n";
        req.ParseRequestChunk(raw);

        // --- Step 2: Define CGI-related configuration ---
        LocationConfig loc;
        loc.cgiExtension = ".py";
        loc.cgiPath = "/usr/bin/python3";
        loc.root = "./www";

        std::string scriptPath = "./www/cgi-bin/hello.py";

        // --- Step 3: Create CGIHandler instance properly ---
        CGIHandler cgi(scriptPath, req, loc);

        // --- Step 4: Execute the CGI script ---
        std::string result = cgi.Execute();

        // --- Step 5: Output the result ---
        std::cout << "----- CGI OUTPUT START -----\n"
                  << result
                  << "\n----- CGI OUTPUT END -----\n";

        std::cout << "✅ CGI test completed successfully.\n";
    }
    catch (const std::exception &e) {
        std::cerr << "❌ CGI test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
