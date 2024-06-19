#ifndef HEADER_FIELDS_HPP
#define HEADER_FIELDS_HPP

#include <map>

#ifndef CR
#define CR "\r"
#endif

#ifndef LF
#define LF "\n"
#endif

#ifndef CRLF
#define CRLF "\r\n"
#endif

#ifndef KEY
#define KEY first
#endif

#ifndef VALUE
#define VALUE second
#endif

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
    void addField(const std::string& key, const int value);
    bool hasField(const std::string& key) const;
    std::string getField(const std::string& key) const;
    std::string toString() const;
};

#endif
