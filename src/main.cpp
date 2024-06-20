#include <iostream>
#include "Config.hpp"
#include "Exception.hpp"
#include "Server.hpp"

int main(int argc, char* argv[])
{
    // Config 기본 경로 설정
    std::string ConfigPath = "default.conf";

    // Config 파일 경로 설정
    if (argc > 1)
    {
        ConfigPath = argv[1];
    }

    try
    {
        Config config(ConfigPath);
        Server server(config);
        server.run();
    }
    catch (const std::exception& e)
    {
        // 서버 실행 중 에러 발생 시 종료
        std::cerr << "Server run failed: " << e.what() << std::endl;
    }

    return 0;
}
