#include "HeaderFields.hpp"
#include <sstream>

HeaderFields::HeaderFields()
: mFields()
{
}
HeaderFields::~HeaderFields()
{
}
void HeaderFields::parseHeaderFields(std::istringstream& headerFields)
{
    std::string line;
    while (std::getline(headerFields, line) && line != "")
    {
        std::string key;
        std::string value;
        std::istringstream lineStream(line);
        std::getline(lineStream, key, ':');
        std::getline(lineStream, value);
        addField(key, value);
    }
}
void HeaderFields::addField(const std::string& key, const std::string& value)
{
    mFields[key] = value;
}
bool HeaderFields::hasField(const std::string& key) const
{
    return mFields.find(key) != mFields.end();
}
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
