#include "lrc.h"
#include "store-type.h"
#include <cstdint>
#include <string>
#include <vector>

enum class MessageType : std::uint32_t {
    NOTHING,
    // PING and PONG are for testing
    PING,
    PONG,

    // ask for information about a track
    GET_TRACK_INFO,
    // respond to GET_TRACK_INFO
    NO_SUCH_TRACK,
    RETURN_TRACK_INFO,
    // ask for a lyrics file
    GET_LYRICS,
    // respond to GET_LYRICS
    NO_SUCH_LYRICS,
    RETURN_LYRICS
};

// the body of MessageType::GET_TRACK_INFO
struct GetTrackInfo {
    std::string title;
};

struct ReturnTrackInfo {
    std::vector<Track> tracks;
    std::string title;
};

struct NoSuchTrack {
    std::string title;
};

struct GetLyrics {
    std::string filename;
};

struct ReturnLyrics {
    Lrc lyrics;
    std::string filename;
};

struct NoSuchLyrics {
    std::string filename;
};

constexpr std::string_view get_message_name(MessageType mt) {
    switch (mt) {
    case MessageType::PING:
        return "PING";
    case MessageType::PONG:
        return "PONG";
    case MessageType::GET_TRACK_INFO:
        return "GET_TRACK_INFO";
    case MessageType::NO_SUCH_TRACK:
        return "NO_SUCH_TRACK";
    case MessageType::RETURN_TRACK_INFO:
        return "RETURN_TRACK_INFO";
    case MessageType::GET_LYRICS:
        return "GET_LYRICS";
    case MessageType::NO_SUCH_LYRICS:
        return "NO_SUCH_LYRICS";
    case MessageType::RETURN_LYRICS:
        return "RETURN_LYRICS";
    default:
        return "???";
    }
}
