#include "message.h"

Message::Message(MessageType t) : header(t) {}
Message::Message() : header(MessageType::PING) {}

std::size_t Message::size() const { return body.size(); }

std::ostream &operator<<(std::ostream &os, const Message &m) {
    os << "Type: " << get_message_name(m.header.type)
       << " Body size (by header): " << m.header.size
       << " Current body size: " << m.body.size();
    return os;
}

void Message::reset() {
   body.clear();
   header.size = 0;
   header.type = MessageType::NOTHING;
}

MessageHeader::MessageHeader(MessageType t) : type(t) {}
