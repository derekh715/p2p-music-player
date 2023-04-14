#include "client.h"

Client::Client(uint16_t port, const std::string &filename): s(false, filename), BaseClient(port) {

}

void Client::populate_tracks() {
    // add this to the database
    Track t {
        .id = 3,
        .album = "Example Album",
        .artist = "Example Author",
        .title = "Example Title",
        .len = 3500
    };
    s.create(t);
}

// VERY IMPORTANT
// this function parses the incoming messages!!!
//
void Client::handle_message(MessageWithOwner &t) {
    switch (t.msg.header.type) {
        case MessageType::PING: {
            std::cout << "received PING from (" << t.id << ")" << std::endl;
            // this one send a PONG response to that computer
            Message resp(MessageType::PONG);
            push_message(t.id, resp);
            break;
        }
        case MessageType::PONG: {
            std::cout << "received PONG from (" << t.id << ")" << std::endl;
            // uncomment this line to see they go back and forth indefinitely
            // Message resp(MessageType::PING);
            // push_message(t.id, resp);
            break;
        }
        case MessageType::NO_SUCH_TRACK: {
            std::cout << "peer (" << t.id << ") says he doesn't have such track." << std::endl;
            break;
        }
        case MessageType::RETURN_TRACK_INFO: {
            std::cout << "peer (" << t.id << ") has this track!" << std::endl;
            ReturnTrackInfo ti;
            t.msg >> ti;
            // print the track info out
            std::cout << ti.t << std::endl;
            break;
        }
        case MessageType::GET_TRACK_INFO: {
            GetTrackInfo gti;
            t.msg >> gti;
            std::cout << "peer (" << t.id << ") asks for track: " << gti.title << std::endl;
            if (gti.title.empty()) {
                std::cout << "The query is empty, returning no such track." << std::endl;
                Message m(MessageType::NO_SUCH_TRACK);
                push_message(t.id, m);
                return;
            }
            auto results = s.search(gti.title);
            if (results.empty()) {
                std::cout << "Couldn't find track. Tell peer that I do not have it." << std::endl;
                Message m(MessageType::NO_SUCH_TRACK);
                push_message(t.id, m);
            } else {
                std::cout << "Found it. Tell peer that I do have it." << std::endl;
                // this is broken right now
                Message m(MessageType::RETURN_TRACK_INFO);
                m << results[0];
                push_message(t.id, m);
            }
            break;
        }
        default:
            std::cout << "Message with unknown message type!" << std::endl;
    }
}
