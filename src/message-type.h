#include <cstdint>
#include <string>
#include "store.h"

enum class MessageType : std::uint32_t {
    NOTHING,
    // PING and PONG are for testing
    PING,
    PONG,

    // ask for information about a track
    GET_TRACK_INFO,
    // respond to GET_TRACK_INFO
    NO_SUCH_TRACK,
    TRACK_INFO
};

// the body of MessageType::GET_TRACK_INFO
struct GetTrackInfo {
    char title[100];
};

struct TrackInfo {
    Track t;
};

constexpr std::string_view get_message_name(MessageType mt) {
    switch (mt) {
        case MessageType::PING: return "PING";
        case MessageType::PONG: return "PONG";
        case MessageType::GET_TRACK_INFO: return "GET_TRACK_INFO";
        case MessageType::NO_SUCH_TRACK: return "NO_SUCH_TRACK";
        case MessageType::TRACK_INFO: return "TRACK_INFO";
        default: return "???";
    }
}
