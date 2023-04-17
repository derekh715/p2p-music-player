#include "client.h"

Client::Client(uint16_t port, const std::string &filename)
    : s(false, filename), BaseClient(port) {}

Client::~Client() {}

std::map<peer_id, std::shared_ptr<tcp::socket>> Client::get_peers() {
    return peers;
}

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

void Client::handle_prepare_picture_sharing(MessageWithOwner &t) {
    reset_sharing_file();
    // prepare chunked file
    PreparePictureSharing pps;
    t.msg >> pps;
    std::cout << "Client " << t.id << " wants file " << pps.which_one
              << std::endl;
    assigned_peer_id = pps.assigned_id_for_peer;

    // change this path if you start the application in a different
    // directory
    fs::path p("./src/tests/data");
    char filename[30];
    sprintf(filename, "1-%d.bmp", pps.which_one);
    p /= filename;
    cf.open_file(p);
    if (cf.failure()) {
        return;
    }
    Message m(MessageType::PREPARED_PICTURE_SHARING);
    PreparedPictureSharing pps2;
    pps2.total_segments = cf.total_segments;
    m << pps2;
    push_message(t.id, m);
}

void Client::handle_prepared_picture_sharing(MessageWithOwner &t) {
    // we know that the peer will engage in sharing that file
    std::cout << "Client " << t.id << " is ready to send the file."
              << std::endl;
    queue_buffer.push_back(std::queue<ReturnPictureSegment>());
    PreparedPictureSharing pps;
    t.msg >> pps;
    total_segment_count = pps.total_segments;
    Message m(MessageType::GET_PICTURE_SEGMENT);
    GetPictureSegment gps;
    gps.segment_id = current_segment_id++;
    m << gps;
    push_message(t.id, m);
}

void Client::handle_return_picture_segment(MessageWithOwner &t) {
    ReturnPictureSegment rps;
    t.msg >> rps;
    std::cout << "Segment " << rps.segment_id << "/" << total_segment_count - 1
              << " received from client " << t.id
              << " share id: " << rps.assigned_id_for_peer << std::endl;
    queue_buffer[rps.assigned_id_for_peer].push(rps);
    Message m(MessageType::GET_PICTURE_SEGMENT);
    // it is enough
    if (rps.segment_id >= total_segment_count) {
        return;
    }
    GetPictureSegment gps;
    gps.segment_id = current_segment_id++;
    m << gps;
    push_message(t.id, m);
}

void Client::handle_get_picture_segment(MessageWithOwner &t) {
    GetPictureSegment gps;
    t.msg >> gps;
    std::cout << "Segment " << gps.segment_id << "/" << cf.total_segments - 1
              << " requested by client " << t.id << std::endl;
    // all requests needed are made
    if (gps.segment_id >= cf.total_segments) {
        return;
    }
    // I do not need this?
    // rps.assigned_id_for_peer = 1;
    ReturnPictureSegment rps;
    bool fine = cf.get(gps.segment_id, rps.body);
    if (!fine) {
        std::cout << "I cannot get this segment!" << std::endl;
        NoSuchPictureSegment nsps;
        // nsps.assigned_id_for_peer = 1;
        nsps.segment_id = gps.segment_id;
        Message m(MessageType::NO_SUCH_PICTURE_SEGMENT);
        m << nsps;
    } else {
        std::cout << "Giving the segment back to client " << t.id << std::endl;
        Message m(MessageType::RETURN_PICTURE_SEGMENT);
        rps.segment_id = gps.segment_id;
        rps.assigned_id_for_peer = assigned_peer_id;
        m << rps;
        push_message(t.id, m);
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
    case MessageType::PREPARE_PICTURE_SHARING:
        handle_prepare_picture_sharing(t);
        break;
    case MessageType::PREPARED_PICTURE_SHARING:
        handle_prepared_picture_sharing(t);
        break;
    case MessageType::RETURN_PICTURE_SEGMENT:
        handle_return_picture_segment(t);
        break;
    case MessageType::GET_PICTURE_SEGMENT:
        handle_get_picture_segment(t);
        break;
    default:
        std::cout << "Message with unknown message type!" << std::endl;
    }
}

void Client::open_file_for_writing() {
    os.open("./interleaved.bmp",
            std::ios::out | std::ios::binary | std::ios::trunc);
    if (!os.is_open()) {
        std::cout << "For some reason the file cannot be opened for writing!"
                  << std::endl;
    }
}

void Client::reset_sharing_file() {
    current_segment_id = 0;
    total_segment_count = 0;
    current_byte = 0;
    current_assigned_id = 0;
}

void Client::additional_cycle_hook() { try_writing_segment(); }

void Client::write_segment(const ReturnPictureSegment &rps) {
    std::cout << "Writing byte: " << rps.body.size() << " to the file ("
              << current_byte << " to " << current_byte + rps.body.size() << ")"
              << std::endl;
    current_byte += rps.body.size();
    os.write(rps.body.data(), rps.body.size());
}

void Client::try_writing_segment() {
    // try to write two segments if possible
    for (int i = 0; i < 1; i++) {
        // traverse through the list to see if there are in-order segments
        // if yes write it to the output stream
        for (auto &q : queue_buffer) {
            if (q.empty()) {
                continue;
            }
            auto &rps = q.front();
            std::cout << "Writing " << current_writing_id
                      << " Total: " << total_segment_count << std::endl;
            if (rps.segment_id == current_writing_id) {
                write_segment(rps);
                q.pop();
                // all requests needed are made, exit now
                if (++current_writing_id >= total_segment_count) {
                    if (os.is_open()) {
                        std::cout
                            << "All segments are received, closing ofstream"
                            << std::endl;
                        os.close();
                    }
                    return;
                }
            }
        }
    }
}
