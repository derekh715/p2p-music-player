#ifndef MESSAGE_H
#define MESSAGE_H

#include "message-type.h"
#include "util.h"
#include <asio.hpp>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

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

    // specialized template for std::vector
    template <typename V>
    friend Message &operator<<(Message &m, const std::vector<V> &d) {
        for (auto &item : d) {
            m << item;
        }
        // also write the size
        m << d.size();
        return m;
    }

    template <typename V>
    friend Message &operator>>(Message &m, std::vector<V> &d) {
        std::size_t size;
        m >> size;
        d.resize(size);
        // there is nothing to push into the vector
        if (size == 0) {
            return m;
        }
        for (int i = size - 1; i >= 0; i--) {
            V item;
            m >> item;
            d[i] = item;
        }
        return m;
    }

    // specialized template for std::string
    friend Message &operator<<(Message &m, const std::string &d);
    friend Message &operator>>(Message &m, std::string &d);

    // specialized template for track info
    friend Message &operator<<(Message &m, const Track &d);
    friend Message &operator>>(Message &m, Track &d);

    friend Message &operator<<(Message &m, const ReturnDatabase &d);
    friend Message &operator>>(Message &m, ReturnDatabase &d);

    // specialized template for get track info struct
    friend Message &operator<<(Message &m, const GetTrackInfo &d);
    friend Message &operator>>(Message &m, GetTrackInfo &d);

    // specialized template for return track info struct
    friend Message &operator<<(Message &m, const ReturnTrackInfo &d);
    friend Message &operator>>(Message &m, ReturnTrackInfo &d);

    // specialized template for get lyrics struct
    friend Message &operator<<(Message &m, const GetLyrics &d);
    friend Message &operator>>(Message &m, GetLyrics &d);

    // specialized template for return lyrics info struct
    friend Message &operator<<(Message &m, const ReturnLyrics &d);
    friend Message &operator>>(Message &m, ReturnLyrics &d);

    // specialized template for lyrics struct
    friend Message &operator<<(Message &m, const Lyric &d);
    friend Message &operator>>(Message &m, Lyric &d);

    // specialized template for LRC struct
    friend Message &operator<<(Message &m, const Lrc &d);
    friend Message &operator>>(Message &m, Lrc &d);

    // specialized template for no such track
    friend Message &operator<<(Message &m, const NoSuchTrack &d);
    friend Message &operator>>(Message &m, NoSuchTrack &d);

    // specialized template for no such lyrics
    friend Message &operator<<(Message &m, const NoSuchLyrics &d);
    friend Message &operator>>(Message &m, NoSuchLyrics &d);

    // all kinds of overoload for interleave
    friend Message &operator<<(Message &m, const PreparePictureSharing &d);
    friend Message &operator>>(Message &m, PreparePictureSharing &d);

    friend Message &operator<<(Message &m, const PreparedPictureSharing &d);
    friend Message &operator>>(Message &m, PreparedPictureSharing &d);

    friend Message &operator<<(Message &m, const GetPictureSegment &d);
    friend Message &operator>>(Message &m, GetPictureSegment &d);

    friend Message &operator<<(Message &m, const ReturnPictureSegment &d);
    friend Message &operator>>(Message &m, ReturnPictureSegment &d);

    friend Message &operator<<(Message &m, const NoSuchPictureSegment &d);
    friend Message &operator>>(Message &m, NoSuchPictureSegment &d);

    friend Message &operator<<(Message &m, const GetAudioFile &d);
    friend Message &operator>>(Message &m, GetAudioFile &d);

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
