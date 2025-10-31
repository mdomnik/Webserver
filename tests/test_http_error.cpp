#include "../HTTP/HTTPResponse/inc/HTTPResponse.hpp"
#include "../Config/inc/ServerConfig.hpp"
#include <iostream>

int main() {
    std::cout << "🧩 HTTP Error Response Test (subject-compliant)\n";

    HTTPResponse res;
    ServerConfig cfg;

    // --- Test 404 Error Page ---
    res.SetResponseToError(404, "HTTP/1.1", "Not Found");
    std::string output404 = res.ResponseToString();
    std::cout << "----- 404 Response -----\n" << output404 << "\n-------------------------\n";

    // --- Test 500 Error Page ---
    res.SetResponseToError(500, "HTTP/1.1", "Internal Server Error");
    std::string output500 = res.ResponseToString();
    std::cout << "----- 500 Response -----\n" << output500 << "\n-------------------------\n";

    std::cout << "✅ HTTP Error Response Test Complete\n";
    return 0;
}
