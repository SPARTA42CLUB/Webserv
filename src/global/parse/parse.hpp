#ifndef PARSE_HPP
#define PARSE_HPP

#include <string>

bool isWhitespace(char c);
void trim(std::string &str);
void pop_back(std::string &str);
bool isDigitStr(const std::string &str);
bool isValidValue(std::string& value);

#endif
