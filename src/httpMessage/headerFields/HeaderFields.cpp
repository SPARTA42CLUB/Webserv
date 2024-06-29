#include "HeaderFields.hpp"
#include <sstream>
#include "HttpException.hpp"

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
    std::getline(headerFields, line, *LF);
    if (line.empty() || line == CR)
    {
        throw HttpException(BAD_REQUEST);
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
            throw HttpException(BAD_REQUEST);
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
    result += "\r\n";
    return result;
}
