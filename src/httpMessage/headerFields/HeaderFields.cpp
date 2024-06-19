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
void HeaderFields::parseHeaderFields(std::istringstream& headerFields)
{
    /*
    key: value\r\n
    key: value\r\n
     */
    std::string line;
    std::getline(headerFields, line);
    if (line.empty() || line == CR)
    {
        throw HTTPException(BAD_REQUEST);
    }
    while (!line.empty() && line != CR) // "\r\n" 혹은 "\n"이 나올 때까지 읽음
    {
        std::string key;
        std::string value;
        std::istringstream lineStream(line);
        std::getline(lineStream, key, ':');
        lineStream >> std::ws; // Skip leading whitespace
        std::getline(lineStream, value, *LF);
        if (value.back() == *CR)
        {
            value.pop_back();
        }
        addField(key, value);
        std::getline(headerFields, line);
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
