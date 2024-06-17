#ifndef MESSAGE_BODY_HPP
#define MESSAGE_BODY_HPP

#include <string>
#include <sstream>

class MessageBody
{
private:
    std::string mContent;
    MessageBody(const MessageBody& rhs);
    MessageBody& operator=(const MessageBody& rhs);
public:
    MessageBody();
    ~MessageBody();
    void parseMessageBody(std::istringstream& body);
    void addBody(const std::string& body);
    const std::string& toString() const;
};

#endif
