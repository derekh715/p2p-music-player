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

Message &operator<<(Message &m, const std::string &d) {
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

Message &operator>>(Message &m, std::string &d) {
    auto start = m.size() - sizeof(std::size_t);
    size_t len;
    std::memcpy(&len, m.body.data() + start, sizeof(std::size_t));
    d.insert(0, m.body.data() + start - len, len);
    m.body.resize(start - len);
    m.header.size = m.size();
    return m;
}

Message &operator<<(Message &m, const Track &d) {
    m << d.id << d.album << d.artist << d.author << d.title << d.lrcfile
      << d.len;
    return m;
}

Message &operator>>(Message &m, Track &d) {
    m >> d.len >> d.lrcfile >> d.title >> d.author >> d.artist >> d.album >>
        d.id;
    return m;
}

Message &operator<<(Message &m, const GetTrackInfo &d) {
    m << d.title;
    return m;
}

Message &operator>>(Message &m, GetTrackInfo &d) {
    m >> d.title;
    return m;
}

Message &operator<<(Message &m, const GetLyrics &d) {
    m << d.filename;
    return m;
}

Message &operator>>(Message &m, GetLyrics &d) {
    m >> d.filename;
    return m;
}

Message &operator<<(Message &m, const ReturnTrackInfo &d) {
    m << d.tracks << d.title;
    return m;
}

Message &operator>>(Message &m, ReturnTrackInfo &d) {
    m >> d.title >> d.tracks;
    return m;
}

Message &operator<<(Message &m, const ReturnLyrics &d) {
    m << d.lyrics << d.filename;
    return m;
}

Message &operator>>(Message &m, ReturnLyrics &d) {
    m >> d.filename >> d.lyrics;
    return m;
}

Message &operator<<(Message &m, const Lrc &d) {
    m << d.album << d.artist << d.author << d.title << d.length << d.creator
      << d.offset << d.editor << d.version << d.lys << d.failure;
    return m;
}
Message &operator>>(Message &m, Lrc &d) {
    m >> d.failure >> d.lys >> d.version >> d.editor >> d.offset >> d.creator >>
        d.length >> d.title >> d.author >> d.artist >> d.album;
    return m;
}

Message &operator<<(Message &m, const Lyric &d) {
    m << d.startms << d.s1 << d.color << d.s2 << d.endms;
    return m;
}

Message &operator>>(Message &m, Lyric &d) {
    m >> d.endms >> d.s2 >> d.color >> d.s1 >> d.startms;
    return m;
}

Message &operator<<(Message &m, const NoSuchTrack &d) {
    m << d.title;
    return m;
}

Message &operator>>(Message &m, NoSuchTrack &d) {
    m >> d.title;
    return m;
}

Message &operator<<(Message &m, const NoSuchLyrics &d) {
    m << d.filename;
    return m;
}

Message &operator>>(Message &m, NoSuchLyrics &d) {
    m >> d.filename;
    return m;
}

MessageHeader::MessageHeader(MessageType t) : type(t) {}
