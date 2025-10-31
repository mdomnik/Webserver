#include "../Server/inc/Server.hpp"
#include "../Config/inc/ConfigParser.hpp"
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "🧩 Server Initialization Test\n";
    std::string file = (argc > 1) ? argv[1] : "ConfigFiles/basic.conf";

    try {
        ConfigParser parser(file);
        std::vector<ServerConfig> cfgs = parser.parse();
        Server srv(cfgs[0]);
        srv.Start();
        std::cout << "✅ Server started successfully\n";
        srv.Stop();
    } catch (const std::exception &e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
