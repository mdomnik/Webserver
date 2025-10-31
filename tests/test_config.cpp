#include "../Config/inc/ConfigParser.hpp"
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "🧩 Config Parser Test\n";
    std::string file = (argc > 1) ? argv[1] : "ConfigFiles/basic.conf";

    try {
        ConfigParser parser(file);
        std::vector<ServerConfig> servers = parser.parse();

        std::cout << "✅ Parsed " << servers.size() << " server blocks\n";
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Server: " << servers[i].serverName
                      << " | Listens on: "
                      << servers[i].listens[0].first << ":" << servers[i].listens[0].second
                      << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "❌ Config parse error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
