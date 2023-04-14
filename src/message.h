#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"
#include <asio.hpp>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>
#include "message-type.h"


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
    std::vector<char> body;

    // mainly for debugging
    friend std::ostream &operator<<(std::ostream &os, const Message &m);

    void reset();


    template <typename Data>
    friend Message &operator<<(Message &m, const Data &d) {
        static_assert(std::is_standard_layout<Data>::value,
                      "This data type cannot be serialized");
        auto end = m.body.size();
        m.body.resize(end + sizeof(Data));
        m.header.size = m.size();
        std::memcpy(m.body.data() + end, &d, sizeof(Data));
        return m;
    }

    template <typename Data> friend Message &operator>>(Message &m, Data &d) {
        static_assert(std::is_standard_layout<Data>::value,
                      "This data type cannot be deserialized");
        auto start = m.size() - sizeof(Data);
        std::memcpy(&d, m.body.data() + start, sizeof(Data));
        m.body.resize(start);
        m.header.size = m.size();
        return m;
    }

    // specialized template for std::string

    friend Message &operator<<(Message &m, const std::string &d) {
        auto end = m.body.size();
        auto len = d.size();
        m.body.resize(end + len + sizeof(std::size_t));
        m.header.size = m.size();
        // write the string
        std::memcpy(m.body.data() + end, d.data(), len);
        // write the string size
        std::memcpy(m.body.data() + end + len, &len, sizeof(std::size_t));
        return m;
    }

    friend Message &operator>>(Message &m, std::string &d) {
        auto start = m.size() - sizeof(std::size_t);
        size_t len;
        std::memcpy(&len, m.body.data() + start, sizeof(std::size_t));
        d.insert(0, m.body.data() + start - len, len);
        m.body.resize(start - len);
        m.header.size = m.size();
        return m;
    }

    // specialized template for track info
    friend Message &operator<<(Message &m, const Track &d) {
        m << d.id << d.album <<  d.artist << d.author << d.title << d.lrcfile << d.len;
        return m;
    }

    friend Message &operator>>(Message &m, Track &d) {
        m >> d.len >> d.lrcfile >> d.title >> d.author >> d.artist >> d.album >> d.id;
        return m;
    }

    // specialized template for get track info struct
    friend Message &operator<<(Message &m, const GetTrackInfo &d) {
        m << d.title;
        return m;
    }

    friend Message &operator>>(Message &m, GetTrackInfo &d) {
        m >> d.title;
        return m;
    }

    // specialized template for return track info struct
    friend Message &operator<<(Message &m, const ReturnTrackInfo &d) {
        m << d.t;
        return m;
    }

    friend Message &operator>>(Message &m, ReturnTrackInfo &d) {
        m >> d.t;
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
