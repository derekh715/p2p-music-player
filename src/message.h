#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"
#include <asio.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

enum class MessageType : std::uint32_t { PING };

class MessageHeader {
  public:
    MessageHeader(MessageType t);

    MessageType type;
    std::uint32_t size;
};

class Message {
  public:
    Message(MessageType t);
    Message();
    std::size_t size() const;
    std::vector<uint8_t> body;

    friend std::ostream &operator<<(std::ostream &os, Message &m);

    template <typename Data>
    friend Message &operator<<(Message &m, const Data &d) {
        static_assert(std::is_standard_layout<Data>::value,
                      "This data type cannot be serialized");
        auto end = m.size();
        m.body.resize(end + sizeof(d));
        m.header.size = m.size();
        std::memcpy(m.body.data() + end, &d, sizeof(d));
        return m;
    }

    template <typename Data> friend Message &operator>>(Message &m, Data &d) {
        static_assert(std::is_standard_layout<Data>::value,
                      "This data type cannot be deserialized");
        auto start = m.size() - sizeof(d);
        std::memcpy(&d, m.body.data() + start, sizeof(d));
        m.body.resize(start);
        m.header.size = m.size();
        return m;
    }

    MessageHeader header;
};

struct MessageWithOwner {
    Message msg;
    peer_id id;
};

#endif
