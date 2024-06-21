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
    size_t start = pos;
    while (pos < reqBody.size() && reqBody[pos] != '\r')
    {
        pos++;
    }
    std::cout << "Test: " << pos - start << std::endl;
    std::string line = reqBody.substr(start, pos - start);
    pos += 2; // Skip '\r\n'
    return line;
}

size_t ChunkedRequestReader::getChunkSize()
{
    size_t start = 0;
    size_t pos = 0;
    while (pos < reqBody.size() && reqBody[pos] != '\r')
    {
        pos++;
    }
    std::string line = reqBody.substr(start, pos - start);
    std::stringstream ss;
    ss << std::hex << line;
    ss >> chunkSize;
    return chunkSize;
}

std::string ChunkedRequestReader::readChunk(size_t &pos)
{
    std::string chunkSizeStr = readLine(pos);
    std::cout << "size: " << chunkSizeStr << std::endl;
    std::stringstream ss;
    ss << std::hex << chunkSizeStr;
    ss >> chunkSize;
    std::cout << "convertedSize: " << chunkSize << std::endl;

    if (chunkSize == 0)
    {
        std::cout << "END!" << std::endl;
        return "";
    }
    std::cout << "pos: " << pos << std::endl;
    std::cout << "chunkSize: " << chunkSize << std::endl;
    std::string chunkData = reqBody.substr(pos, chunkSize);
    std::cout << "reqBodySize: " << reqBody.size() << std::endl;
    std::cout << "readChunkSize: " << chunkData.size() << std::endl;
    pos += chunkSize + 2; // Move past the chunk data and the trailing "\r\n"
    return chunkData;
}

bool ChunkedRequestReader::processRequest()
{
	// std::cout << "Processing chunked request..." << std::endl;
    // std::ofstream outFile(uploadFilePath, std::ios::binary);
	std::ofstream outFile(uploadFilePath, std::ios::binary | std::ios::app);
    if (!outFile)
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return false;
    }

    size_t pos = 0;
	// std::cout << "Reading chunk..." << std::endl;
    std::string chunkData = readChunk(pos);
    // std::cout << "returnChunkSize: " << chunkData.size() << std::endl;
    if (chunkData.empty())
    {
		outFile.close();
		if (!outFile)
		{
			std::cerr << "Failed to close the file." << std::endl;
		}
		return true;
    }
	// std::cout << "writing chunk to file..." << std::endl;
    outFile.write(chunkData.c_str(), chunkData.size());
	// std::cout << "Chunk written to file." << std::endl;

    outFile.close();
    if (!outFile)
    {
        std::cerr << "Failed to close the file." << std::endl;
    }
	return false;
}
