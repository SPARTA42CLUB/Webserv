#ifndef RANGE_REQUEST_READER_HPP
#define RANGE_REQUEST_READER_HPP

#include <string>
#include <vector>

class RangeRequestReader
{
private:
    std::string filePath;
    std::vector<std::pair<size_t, size_t> > ranges;

    std::string readRange(size_t start, size_t end);

public:
    RangeRequestReader(const std::string &filePath);
    ~RangeRequestReader();

    void addRange(size_t start, size_t end);
    std::string processRequest();
};

#endif
