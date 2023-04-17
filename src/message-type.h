#include "lrc.h"
#include "store-types.h"
#include "util.h"
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
    RETURN_LYRICS,

    // file streaming
    GET_AUDIO_FILE,
    PREPARE_AUDIO_SHARING,
    PREPARED_AUDIO_SHARING,
    NO_SUCH_AUDIO_FILE,
    HAS_AUDIO_FILE,
    GET_AUDIO_SEGMENT,
    RETURN_AUDIO_SEGMENT,

    // the interleaving pictures example
    PREPARE_PICTURE_SHARING,
    PREPARED_PICTURE_SHARING,
    HAS_PICTURE_FILE,
    GET_PICTURE_SEGMENT,
    RETURN_PICTURE_SEGMENT,
    NO_SUCH_PICTURE_SEGMENT
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

struct PrepareAudioSharing {
    // the id is not local, it refers to the remote database of THAT PEER
    // it is present in ReturnTrackInfo
    int remote_id_for_peer;
};

struct PreparePictureSharing {
    int which_one;
    peer_id assigned_id_for_peer;
};

struct PreparedPictureSharing {
    int total_segments;
};

struct GetPictureSegment {
    int segment_id;
};

struct ReturnPictureSegment {
    int segment_id;
    peer_id assigned_id_for_peer;
    std::vector<char> body;
};

struct NoSuchPictureSegment {
    int segment_id;
    peer_id assigned_id_for_peer;
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
    case MessageType::GET_AUDIO_FILE:
        return "GET_AUDIO_FILE";
    case MessageType::HAS_AUDIO_FILE:
        return "HAS_AUDIO_FILE";
    case MessageType::NO_SUCH_AUDIO_FILE:
        return "NO_SUCH_AUDIO_FILE";
    case MessageType::PREPARE_AUDIO_SHARING:
        return "PREPARE_AUDIO_SHARING";
    case MessageType::PREPARED_AUDIO_SHARING:
        return "PREPARED_AUDIO_SHARING";
    case MessageType::GET_AUDIO_SEGMENT:
        return "GET_AUDIO_SEGMENT";
    case MessageType::RETURN_AUDIO_SEGMENT:
        return "RETURN_AUDIO_SEGMENT";
    case MessageType::PREPARE_PICTURE_SHARING:
        return "PREPARE_PICTURE_SHARING";
    case MessageType::PREPARED_PICTURE_SHARING:
        return "PREPARED_PICTURE_SHARING";
    case MessageType::GET_PICTURE_SEGMENT:
        return "GET_PICTURE_SEGMENT";
    case MessageType::RETURN_PICTURE_SEGMENT:
        return "RETURN_PICTURE_SEGMENT";
    case MessageType::NO_SUCH_PICTURE_SEGMENT:
        return "NO_SUCH_PICTURE_SEGMENT";
    default:
        return "???";
    }
}
