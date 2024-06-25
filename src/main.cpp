#include <iostream>
#include "Config.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "ConfigException.hpp"
#include "HTTPException.hpp"
#include "SysException.hpp"

static std::string makeConfigPath(int argc, char* argv[], bool& isOption);

int main(int argc, char* argv[])
{
    bool isOption = false;
    const std::string ConfigPath = makeConfigPath(argc, argv, isOption);

    const Config config(ConfigPath);
    if (isOption)
    {
        config.print();
        return 0;
    }
    Server server(config);
    try
    {
        server.run();
    }
    catch (const std::exception& e)
    {
        // 서버 실행 중 에러 발생 시 종료
        Logger::getInstance().logError(e.what());
    }

    return 0;
}

static std::string makeConfigPath(int argc, char* argv[], bool& isOption)
{
    std::string configPath = "conf/default.conf";
    if (argc > 1)
    {
        if (std::string(argv[1]) == "-c" || std::string(argv[1]) == "--config")
        {
            isOption = true;
            if (argc > 2)
            {
                configPath = argv[2];
            }
        }
        else
        {
            configPath = argv[1];
        }
    }
    return configPath;
}
