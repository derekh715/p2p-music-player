#include "message.h"

Message::Message(MessageType t) : header(t) {}
Message::Message() : header(MessageType::PING) {}

std::size_t Message::size() const { return body.size(); }

std::ostream &operator<<(std::ostream &os, Message &m) {
    os << "Type : " << static_cast<std::uint32_t>(m.header.type)
       << " Size: " << m.size();
    return os;
}

MessageHeader::MessageHeader(MessageType t) : type(t) {}
