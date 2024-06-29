#ifndef MESSAGE_BODY_HPP
#define MESSAGE_BODY_HPP

#include <sstream>
#include <string>

class MessageBody
{
private:
    std::string mContent;
    MessageBody(const MessageBody& rhs);
    MessageBody& operator=(const MessageBody& rhs);

public:
    MessageBody();
    ~MessageBody();
    void addBody(const std::string& body);
    const std::string& toString() const;
    size_t size() const;
    void clear();
};

#endif
