#include "MessageBody.hpp"

MessageBody::MessageBody()
: mContent("")
{
}
MessageBody::~MessageBody()
{
}
void MessageBody::parseMessageBody(std::istringstream& body)
{
    std::string line;
    while (std::getline(body, line))
    {
        // TODO: 마지막 줄에 개행이 없을 경우 처리
        mContent += line + "\n";
    }
}
void MessageBody::addBody(const std::string& body)
{
    mContent += body;
}
const std::string& MessageBody::toString() const
{
    return mContent;
}
size_t MessageBody::size() const
{
    return mContent.size();
}
void MessageBody::clear()
{
    mContent.clear();
}
