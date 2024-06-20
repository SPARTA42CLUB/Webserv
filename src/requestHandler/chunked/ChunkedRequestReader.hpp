#ifndef CHUNKED_REQUEST_READER_HPP
#define CHUNKED_REQUEST_READER_HPP

#include <string>
#include "ChunkedBuffer.hpp"

class ChunkedRequestReader
{
private:
    const std::string uploadFilePath;
    const std::string reqBody;
    ChunkedBuffer buffer;

    // Method to read a line from the request body
    std::string readLine(size_t &pos);

    // Method to read a chunk of data from the request body
    std::string readChunk(size_t &pos);

public:
    ChunkedRequestReader(const std::string &uploadFilePath, const std::string &reqBody);
    ~ChunkedRequestReader();

    // Method to process the chunked request
	// NOTE: This method should be called repeatedly until it returns false
    bool processRequest();
};

#endif
