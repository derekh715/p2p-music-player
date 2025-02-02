#include "client.h"

Client::Client(uint16_t port, const std::string &filename)
    : s(false, filename), BaseClient(port, 750ms) {
    cycle();
}

Client::~Client() {}

void Client::on_connect(peer_id id) {
    push_message(id, Message(MessageType::PING));
}

void Client::on_disconnect(peer_id id) {
    std::cout << "Disconnected!" << std::endl;
}

void Client::populate_tracks() {
    // add this to the database (for the example network application)
    Track t{
        .id = 3,
        .album = "Example Album",
        .artist = "Example Author",
        .title = "Example Title",
        .duration = 3500,
        .checksum = "a3344",
    };
    Track t1{
        .id = 4,
        .album = "Another Example Album",
        .artist = "Another Example Author",
        .title = "Another Example Title",
        .duration = 3500,
        .checksum = "a3345",
    };
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
        NoSuchTrack nst;
        nst.title = gti.title;
        m << nst;
        push_message(t.id, m);
        return;
    }
    auto results = s.search(gti.title);
    if (results.empty()) {
        std::cout << "Couldn't find track. Tell peer that I do not have it."
                  << std::endl;
        Message m(MessageType::NO_SUCH_TRACK);
        NoSuchTrack nst;
        nst.title = gti.title;
        m << nst;
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
        NoSuchLyrics nsl;
        nsl.filename = gl.filename;
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
    // prepare chunked file
    PrepareFileSharing pfs;
    t.msg >> pfs;
    std::cout << "Client " << t.id << " wants file " << pfs.name << std::endl;
    assigned_peer_id = pfs.assigned_id_for_peer;

    // send more things at once
    std::cout << "Dictated segment count: " << pfs.dictated_segment_count
              << std::endl;
    if (pfs.dictated_segment_count > 0) {
        cf.open_file_with_segment_count(pfs.name, pfs.dictated_segment_count);
    } else {
        cf.open_file(pfs.name, DEFAULT_CHUNK_SIZE * 2);
    }
    if (cf.failure()) {
        return;
    }
    Message m(MessageType::PREPARED_FILE_SHARING);
    PreparedFileSharing pps2;
    pps2.total_segments = cf.total_segments;
    pps2.assigned_id_for_peer = assigned_peer_id;
    m << pps2;
    push_message(t.id, m);
}

void Client::handle_prepared_picture_sharing(MessageWithOwner &t) {
    // we know that the peer will engage in sharing that file
    std::cout << "Client " << t.id << " is ready to send the file."
              << std::endl;
    PreparedFileSharing pps;
    t.msg >> pps;
    fs.set_segment_count(pps.total_segments);
    Message m(MessageType::GET_SEGMENT);
    GetSegment gps;
    gps.segment_id = fs.get_next_segment_id();
    m << gps;
    push_message(t.id, m);
    std::cout << "Assigned id is " << pps.assigned_id_for_peer << std::endl;
    fs.start_timeout(pps.assigned_id_for_peer);
}

void Client::handle_return_picture_segment(MessageWithOwner &t) {
    ReturnSegment rps;
    t.msg >> rps;
    fs.end_timeout(rps.assigned_id_for_peer);
    std::cout << "Segment " << rps.segment_id << "/"
              << fs.get_segment_count() - 1 << " received from client " << t.id
              << " share id: " << rps.assigned_id_for_peer << std::endl;
    fs.push_segment(rps);
    std::cout << "Pushed segment to fs! " << std::endl;
    Message m(MessageType::GET_SEGMENT);
    // it is enough
    if (fs.all_segments_asked()) {
        return;
    }
    GetSegment gps;
    gps.segment_id = fs.get_next_segment_id();
    m << gps;
    fs.start_timeout(rps.assigned_id_for_peer);
    push_message(t.id, m);
}

void Client::handle_get_picture_segment(MessageWithOwner &t) {
    GetSegment gps;
    t.msg >> gps;
    std::cout << "Segment " << gps.segment_id << "/" << cf.total_segments - 1
              << " requested by client " << t.id << std::endl;
    // all requests needed are made
    if (gps.segment_id >= cf.total_segments) {
        return;
    }
    ReturnSegment rps;
    bool fine = cf.get(gps.segment_id, rps.body);
    if (!fine) {
        std::cout << "I cannot get this segment!" << std::endl;
        NoSuchSegment nsps;
        nsps.assigned_id_for_peer = assigned_peer_id;
        nsps.segment_id = gps.segment_id;
        Message m(MessageType::NO_SUCH_SEGMENT);
        m << nsps;
    } else {
        std::cout << "Giving the segment back to client " << t.id << std::endl;
        Message m(MessageType::RETURN_SEGMENT);
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
    case MessageType::PREPARE_FILE_SHARING:
        handle_prepare_picture_sharing(t);
        break;
    case MessageType::PREPARED_FILE_SHARING:
        handle_prepared_picture_sharing(t);
        break;
    case MessageType::RETURN_SEGMENT:
        handle_return_picture_segment(t);
        break;
    case MessageType::GET_SEGMENT:
        handle_get_picture_segment(t);
        break;
    case MessageType::GET_DATABASE:
        handle_get_database(t);
        break;
    case MessageType::RETURN_DATABASE:
        handle_return_database(t);
        break;
    default:
        std::cout << "Message with unknown message type!" << std::endl;
    }
}

void Client::additional_cycle_hook() {
    // custom handler for writing a segment
    fs.try_writing_segment([this](const ReturnSegment &rps, bool end) {
        os.write(rps.body.data(), rps.body.size());
        if (end) {
            if (os.is_open()) {
                std::cout << "All segments are received, closing ofstream"
                          << std::endl;
                os.close();
            }
        }
    });
    // if some peer enters the waiting state
    fs.if_idle([this](int assigned_peer_id) {
        Message m(MessageType::GET_SEGMENT);
        // it is enough
        if (fs.all_segments_asked()) {
            return;
        }
        GetSegment gps;
        gps.segment_id = fs.get_next_segment_id();
        m << gps;
        push_message(fs.get_peer_id(assigned_peer_id), m);
    });
}

void Client::start_file_sharing(const std::string &filename) {
    fs.reset_sharing_file();
    os.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!os.is_open()) {
        std::cout << "For some reason the file cannot be opened for writing!"
                  << std::endl;
    }
}

void Client::cycle() {
    timer.expires_from_now(cycle_time);
    timer.async_wait([&](asio::error_code ec) {
        if (ec) {
            std::cout << "[CYCLE] Something is wrong with the timer: "
                      << ec.message() << std::endl;
            return;
        }
        additional_cycle_hook();
        // clear the in messages array first, if there are messages clear them
        while (!in_msgs.empty()) {
            auto msg = in_msgs.pop_front();
            handle_message(msg);
        }
        // now we clear the out messages array
        start_writing();
        cycle();
    });
}

void Client::handle_get_database(MessageWithOwner &t) {
    std::cout << "Client " << t.id << " needs the entire database!"
              << std::endl;
    ReturnDatabase rd;
    Message m(MessageType::RETURN_DATABASE);
    rd.tracks = s.read_all();
    m << rd;
    push_message(t.id, m);
}

void Client::handle_return_database(MessageWithOwner &t) {
    ReturnDatabase rd;
    t.msg >> rd;
    std::cout << "Returning database:" << std::endl;
    for (auto &t : rd.tracks) {
        std::cout << t << std::endl;
    }
}
