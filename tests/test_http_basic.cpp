#include "../HTTP/HTTPResponse/inc/HTTPResponse.hpp"
#include "../HTTP/HTTPRequest/inc/HTTPRequest.hpp"
#include "../Config/inc/ServerConfig.hpp"
#include <iostream>

int main() {
    std::cout << "🧩 HTTP Basic Response Test (using GenerateResponse)\n";

    // === Create a fake GET request ===
    HTTPRequest req;
    std::string rawRequest =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: TestClient\r\n"
        "\r\n";

    req.ParseRequestChunk(rawRequest);

    // === Create a simple config with valid location ===
    ServerConfig cfg;
    LocationConfig loc;
    loc.root = "./www";
    loc.index = "index.html";
    cfg.locations.push_back(loc);

    // === Generate response ===
    HTTPResponse res;
    std::string output = res.GenerateResponse(req, cfg);

    std::cout << "----- HTTP Response -----\n"
              << output
              << "\n--------------------------\n";

    std::cout << "✅ HTTP Basic Response Test Complete\n";
    return 0;
}
