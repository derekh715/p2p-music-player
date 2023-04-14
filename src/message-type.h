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
    RETURN_TRACK_INFO
};

// the body of MessageType::GET_TRACK_INFO
struct GetTrackInfo {
    std::string title;
};

struct ReturnTrackInfo {
    Track t;
};

constexpr std::string_view get_message_name(MessageType mt) {
    switch (mt) {
        case MessageType::PING: return "PING";
        case MessageType::PONG: return "PONG";
        case MessageType::GET_TRACK_INFO: return "GET_TRACK_INFO";
        case MessageType::NO_SUCH_TRACK: return "NO_SUCH_TRACK";
        case MessageType::RETURN_TRACK_INFO: return "RETURN_TRACK_INFO";
        default: return "???";
    }
}
