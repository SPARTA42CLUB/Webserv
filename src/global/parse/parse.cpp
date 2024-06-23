#include "parse.hpp"

void trim(std::string &str)
{
    size_t start = 0;
    size_t end = str.size();
    while (start < end && isWhitespace(str[start]))
    {
        ++start;
    }
    while (end > start && isWhitespace(str[end - 1]))
    {
        --end;
    }
    if (start == end)
    {
        str.clear();
    }
    else
    {
        str = str.substr(start, end - start);
    }
}

bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
