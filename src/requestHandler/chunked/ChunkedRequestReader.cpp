#include "ChunkedRequestReader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

ChunkedRequestReader::ChunkedRequestReader(const std::string &uploadFilePath, const std::string &reqBody)
    : uploadFilePath(uploadFilePath), reqBody(reqBody)
{
}

ChunkedRequestReader::~ChunkedRequestReader()
{
}

std::string ChunkedRequestReader::readLine(size_t &pos)
{
    std::string line;
    while (pos < reqBody.size() && reqBody[pos] != '\r')
    {
        line += reqBody[pos];
        pos++;
    }
    pos += 2; // Skip '\r\n'
    return line;
}

std::string ChunkedRequestReader::readChunk(size_t &pos)
{
    std::string chunkSizeStr = readLine(pos);
    std::stringstream ss;
    ss << std::hex << chunkSizeStr;
    size_t chunkSize;
    ss >> chunkSize;

    if (chunkSize == 0)
    {
        return "";
    }

    std::string chunkData = reqBody.substr(pos, chunkSize);
    pos += chunkSize + 2; // Move past the chunk data and the trailing "\r\n"
    return chunkData;
}

bool ChunkedRequestReader::processRequest()
{
	std::cout << "Processing chunked request..." << std::endl;
    std::ofstream outFile(uploadFilePath, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return false;
    }

    size_t pos = 0;
    while (pos < reqBody.size())
    {
		std::cout << "Reading chunk..." << std::endl;
        std::string chunkData = readChunk(pos);
		std::cout << "Chunk size: " << chunkData.size() << std::endl;
		std::cout << "Chunk data: " << chunkData << std::endl;
        if (chunkData.empty())
        {
            break;
        }
        outFile.write(chunkData.c_str(), chunkData.size());
    }

    outFile.close();
    if (!outFile)
    {
        std::cerr << "Failed to close the file." << std::endl;
    }
	return true;
}
