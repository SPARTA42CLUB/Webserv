#ifndef CHUNKED_BUFFER_HPP
#define CHUNKED_BUFFER_HPP

#include <string>
#include <fstream>
#include <iostream>

class ChunkedBuffer
{
private:
    static const int CHUNK_BUFFER_SIZE = 1024;
    std::string buffer;

public:
    ChunkedBuffer();
    ~ChunkedBuffer();

    // Method to add chunk data to the buffer
    void addChunk(const std::string &chunkData);

    // Method to write buffer to a file
    void writeToFile(const std::string &filename);
};

#endif
