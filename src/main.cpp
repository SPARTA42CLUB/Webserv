#include <iostream>
#include "Config.hpp"
#include "Exception.hpp"
#include "Server.hpp"
#include "color.hpp"

static bool isOption(const std::string& arg);

int main(int argc, char* argv[])
{
    // Config 기본 경로 설정
    std::string ConfigPath = "default.conf";
    if (argc > 1 && !isOption(argv[1]))
        ConfigPath = argv[1];

    try
    {
        Config config(ConfigPath);
        if (argc > 1 && isOption(argv[1]))
        {
            config.print();
            return 0;
        }
        Server server(config);
        server.run();
    }
    catch (const std::exception& e)
    {
        // 서버 실행 중 에러 발생 시 종료
        std::cerr << color::FG_RED << "Server run failed: " << e.what() << color::RESET << std::endl;
    }

    return 0;
}

bool isOption(const std::string& arg)
{
    return (arg == "-c" || arg == "--config");
}
