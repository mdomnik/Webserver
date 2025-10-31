#include "../Server/inc/ServerManager.hpp"
#include "../Config/inc/ConfigParser.hpp"
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "🧩 Full Integration Test\n";
    std::string file = (argc > 1) ? argv[1] : "ConfigFiles/multi_server.conf";

    try {
        ConfigParser parser(file);
        std::vector<ServerConfig> configs = parser.parse();
        ServerManager manager(configs);
        std::cout << "✅ Initialized " << configs.size() << " server(s)\n";
    } catch (const std::exception &e) {
        std::cerr << "❌ Integration error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
