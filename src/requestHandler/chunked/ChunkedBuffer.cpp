#include "ChunkedBuffer.hpp"

ChunkedBuffer::ChunkedBuffer() : buffer("")
{
}

ChunkedBuffer::~ChunkedBuffer()
{
}

void ChunkedBuffer::addChunk(const std::string &chunkData)
{
    buffer += chunkData;
}

void ChunkedBuffer::writeToFile(const std::string &filename)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    outFile.write(buffer.c_str(), buffer.size());

    if (!outFile)
    {
        std::cerr << "Failed to write to the file." << std::endl;
    }

    outFile.close();
    if (!outFile)
    {
        std::cerr << "Failed to close the file." << std::endl;
    }
}