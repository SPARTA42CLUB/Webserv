#include "HeaderFields.hpp"
#include <sstream>
#include "HTTPException.hpp"

HeaderFields::HeaderFields()
: mFields()
{
}
HeaderFields::~HeaderFields()
{
}
#include <iostream>
void HeaderFields::parseHeaderFields(std::istringstream& headerFields)
{
    /*
    key: value\r\n
    key: value\r\n
     */
    std::string line;
    std::getline(headerFields, line, *LF);
    // NOTE: 테스트를 위해 "\n"으로 헤더가 끝나는 경우도 허용
    // FIXME: "\n"으로 헤더가 끝나는 경우 삭제 + telnet으로 테스트 시 34행에서 stack-buffer-overflow 발생
    // FIXME: 원래는 무조건 양식을 지키는 string을 받는 경우에만 reqMsg, resMsg가 생성되었지만 chunked인 경우 양식을 지키지 않아도 객체가 생성되므로 수정 필요
    if (line.empty() || line == CR)
    {
        throw HTTPException(BAD_REQUEST);
    }
    while (!line.empty() && line != CR)  // "\r\n"이나 "\n"이 나올때까지 반복
    {
        std::string key;
        std::string value;
        std::istringstream lineStream(line);
        std::getline(lineStream, key, ':');
        lineStream >> std::ws;  // Skip leading whitespace
        std::getline(lineStream, value, *LF);
        if (value.empty() || value.back() != *CR)
        {
            throw HTTPException(BAD_REQUEST);
        }
        value.pop_back();
        addField(key, value);
        std::getline(headerFields, line, *LF);
    }
}
void HeaderFields::addField(const std::string& key, const std::string& value)
{
    mFields[key] = value;
}
void HeaderFields::addField(const std::string& key, const int value)
{
    mFields[key] = std::to_string(value);
}
bool HeaderFields::hasField(const std::string& key) const
{
    return mFields.find(key) != mFields.end();
}

// 해당 필드가 없을시 "" 반환
std::string HeaderFields::getField(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = mFields.find(key);
    if (it != mFields.end())
    {
        return it->VALUE;
    }
    else
    {
        return "";
    }
}
std::string HeaderFields::toString() const
{
    std::string result;
    for (std::map<std::string, std::string>::const_iterator it = mFields.begin(); it != mFields.end(); ++it)
    {
        result += it->KEY + ": " + it->VALUE + "\r\n";
    }
    return result;
}
