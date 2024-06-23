#include <iostream>
#include "Config.hpp"
#include "Exception.hpp"
#include "Server.hpp"
#include "Logger.hpp"

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
        // TEST: Config 파일 파싱 결과 출력
        // config.print();
        // ~~
        Server server(config);
        server.run();
    }
    catch (const std::exception& e)
    {
        // 서버 실행 중 에러 발생 시 종료
        Logger::getInstance().logError(e.what());
    }

    return 0;
}
