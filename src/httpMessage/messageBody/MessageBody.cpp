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
    while(std::getline(body, line))
    {
        mContent += line;
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
