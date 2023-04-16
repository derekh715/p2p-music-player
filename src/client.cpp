#include "client.h"

Client::Client(uint16_t port, const std::string &filename)
    : s(false, filename), BaseClient(port) {}

void Client::populate_tracks() {
    // add this to the database (for the example network application)
    Track t{.id = 3,
            .album = "Example Album",
            .artist = "Example Author",
            .title = "Example Title",
            .duration = 3500};
    Track t1{.id = 4,
             .album = "Another Example Album",
             .artist = "Another Example Author",
             .title = "Another Example Title",
             .duration = 3500};
    s.create(t);
    s.create(t1);
}

void Client::handle_ping(MessageWithOwner &t) {
    std::cout << "received PING from (" << t.id << ")" << std::endl;
    // this one send a PONG response to that computer
    Message resp(MessageType::PONG);
    push_message(t.id, resp);
}

void Client::handle_pong(MessageWithOwner &t) {
    std::cout << "received PONG from (" << t.id << ")" << std::endl;
    // uncomment this line to see they go back and forth indefinitely
    // Message resp(MessageType::PING);
    // push_message(t.id, resp);
}

void Client::handle_no_such_track(MessageWithOwner &t) {
    NoSuchTrack nst;
    t.msg >> nst;
    std::cout << "peer (" << t.id << ") says he doesn't have track with title"
              << nst.title << std::endl;
}

void Client::handle_return_track_info(MessageWithOwner &t) {
    ReturnTrackInfo ti;
    t.msg >> ti;
    std::cout << "peer (" << t.id << ") has tracks for title " << ti.title
              << "!" << std::endl;
    // print the track info out
    for (auto &t : ti.tracks) {
        std::cout << t << std::endl;
    }
}

void Client::handle_get_track_info(MessageWithOwner &t) {
    GetTrackInfo gti;
    t.msg >> gti;
    std::cout << "peer (" << t.id << ") asks for track: " << gti.title
              << std::endl;
    if (gti.title.empty()) {
        std::cout << "The query is empty, returning no such track."
                  << std::endl;
        Message m(MessageType::NO_SUCH_TRACK);
        push_message(t.id, m);
        return;
    }
    auto results = s.search(gti.title);
    if (results.empty()) {
        std::cout << "Couldn't find track. Tell peer that I do not have it."
                  << std::endl;
        Message m(MessageType::NO_SUCH_TRACK);
        push_message(t.id, m);
    } else {
        std::cout << "Found it. Tell peer that I do have it." << std::endl;
        Message m(MessageType::RETURN_TRACK_INFO);
        // message class allows pushing the vectors into it
        ReturnTrackInfo rti{.tracks = results, .title = gti.title};
        m << rti;
        push_message(t.id, m);
    }
}

void Client::handle_no_such_lyrics(MessageWithOwner &t) {
    NoSuchLyrics nsl;
    t.msg >> nsl;
    std::cout << "Peer (" << t.id
              << ") said he doesn't have a lyric file of path " << nsl.filename
              << std::endl;
}

void Client::handle_get_lyrics(MessageWithOwner &t) {
    GetLyrics gl;
    t.msg >> gl;
    std::cout << "Peer (" << t.id << ") wants to have " << gl.filename
              << std::endl;
    Lrc f(gl.filename.c_str());
    if (f.failed()) {
        NoSuchLyrics nsl{gl.filename};
        Message m(MessageType::NO_SUCH_LYRICS);
        m << nsl;
        push_message(t.id, m);
    } else {
        Message m(MessageType::RETURN_LYRICS);
        ReturnLyrics rl{
            .lyrics = f,
            .filename = gl.filename,
        };
        m << rl;
        push_message(t.id, m);
    }
}

void Client::handle_return_lyrics(MessageWithOwner &t) {
    ReturnLyrics rl;
    t.msg >> rl;
    std::cout << "Peer (" << t.id << ") returns lyrics for " << rl.filename
              << std::endl;
    if (rl.lyrics.failed()) {
        std::cout << "The lyrics file failed for some reason." << std::endl;
        return;
    }

    // example output
    std::cout << "Here are the lines of the lyrics file" << std::endl;
    for (auto &line : rl.lyrics.GetAllLyrics()) {
        std::cout << line.s1 << std::endl;
    }
}

// VERY IMPORTANT
// this function parses the incoming messages!!!
//
void Client::handle_message(MessageWithOwner &t) {
    switch (t.msg.header.type) {
    case MessageType::PING:
        handle_ping(t);
        break;

    case MessageType::PONG:
        handle_pong(t);
        break;

    case MessageType::NO_SUCH_TRACK:
        handle_no_such_track(t);
        break;

    case MessageType::RETURN_TRACK_INFO:
        handle_return_track_info(t);
        break;

    case MessageType::GET_TRACK_INFO:
        handle_get_track_info(t);
        break;
    case MessageType::NO_SUCH_LYRICS:
        handle_no_such_lyrics(t);
        break;
    case MessageType::GET_LYRICS:
        handle_get_lyrics(t);
        break;
    case MessageType::RETURN_LYRICS:
        handle_return_lyrics(t);
        break;
    default:
        std::cout << "Message with unknown message type!" << std::endl;
    }
}
