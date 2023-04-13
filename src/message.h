#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"
#include <asio.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

enum class MessageType : std::uint32_t { PING, PONG };

/*
 * The header fields for every message that is sent in this application
 * type identifies which type of message it is (see MessageType)
 * size gives the size of body (see Message)
 * it is of paramount importance to fill the size field correctly
 * or else the receiver will read the wrong amount of bytes
 */
class MessageHeader {
  public:
    MessageHeader(MessageType t);

    MessageType type;
    std::uint32_t size = 0;
};

/*
 * Represents a message in our protocol
 * use << to push data INTO the body, the size of header will be automatically
 * adjusted use >> to push data OUT OF the body, the size of header will be
 * automatically adjusted
 *
 * For example, we can construct a message with only an integer as body via
 * this:
 * Message m(MessageType::SEND_INTEGER)
 * m << 1;
 *
 * I just made up the SEND_INTEGER type, more types can be added above
 *
 * To read an integer from a MessageType::SEND_INTEGER, we can do:
 * int holder;
 * m >> holder;
 * EXPECT_EQ(holder, 1); // true
 */
class Message {
  public:
    Message(MessageType t);
    Message();
    std::size_t size() const;
    std::vector<uint8_t> body;

    // mainly for debugging
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

/*
 * Message but with a peer_id, used to identify who send the message to you
 */
struct MessageWithOwner {
    Message msg;
    peer_id id;
};

#endif
