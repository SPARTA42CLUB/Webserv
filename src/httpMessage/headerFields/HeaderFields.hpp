#ifndef HEADER_FIELDS_HPP
#define HEADER_FIELDS_HPP

#include <map>

#define KEY first
#define VALUE second

class HeaderFields
{
private:
    std::map<std::string, std::string> mFields;
    HeaderFields(const HeaderFields& rhs);
    const HeaderFields& operator=(const HeaderFields& rhs);
public:
    HeaderFields();
    ~HeaderFields();
    void parseHeaderFields(std::istringstream& headerFields);
    void addField(const std::string& key, const std::string& value);
    bool hasField(const std::string& key) const;
    std::string getField(const std::string& key) const;
    std::string toString() const;
};

#endif
