#include "client.h"

Client::Client(uint16_t port, const std::string &filename)
    : s(false, filename), BaseClient(port, 500ms) {
    cycle();
}

Client::~Client() {}

std::map<peer_id, std::shared_ptr<tcp::socket>> Client::get_peers() {
    return peers;
}

void Client::on_connect(peer_id id) {
    std::cout << "Overridden!!!" << std::endl;
    push_message(id, Message(MessageType::PING));
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
    // send more things at once
    cf.open_file(p, DEFAULT_CHUNK_SIZE * 2);
    if (cf.failure()) {
        return;
    }
    Message m(MessageType::PREPARED_PICTURE_SHARING);
    PreparedPictureSharing pps2;
    pps2.total_segments = cf.total_segments;
    pps2.assigned_id_for_peer = assigned_peer_id;
    m << pps2;
    push_message(t.id, m);
}

void Client::handle_prepared_picture_sharing(MessageWithOwner &t) {
    // we know that the peer will engage in sharing that file
    std::cout << "Client " << t.id << " is ready to send the file."
              << std::endl;
    PreparedPictureSharing pps;
    t.msg >> pps;
    ps.set_segment_count(pps.total_segments);
    Message m(MessageType::GET_PICTURE_SEGMENT);
    GetPictureSegment gps;
    gps.segment_id = ps.get_next_segment_id();
    m << gps;
    push_message(t.id, m);
    std::cout << "Assigned id is " << pps.assigned_id_for_peer << std::endl;
    ps.start_timeout(pps.assigned_id_for_peer);
}

void Client::handle_return_picture_segment(MessageWithOwner &t) {
    ReturnPictureSegment rps;
    t.msg >> rps;
    ps.end_timeout(rps.assigned_id_for_peer);
    std::cout << "Segment " << rps.segment_id << "/"
              << ps.get_segment_count() - 1 << " received from client " << t.id
              << " share id: " << rps.assigned_id_for_peer << std::endl;
    ps.push_segment(rps);
    Message m(MessageType::GET_PICTURE_SEGMENT);
    // it is enough
    if (ps.all_segments_asked()) {
        return;
    }
    GetPictureSegment gps;
    gps.segment_id = ps.get_next_segment_id();
    m << gps;
    ps.start_timeout(rps.assigned_id_for_peer);
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
    ReturnPictureSegment rps;
    bool fine = cf.get(gps.segment_id, rps.body);
    if (!fine) {
        std::cout << "I cannot get this segment!" << std::endl;
        NoSuchPictureSegment nsps;
        nsps.assigned_id_for_peer = assigned_peer_id;
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
    ps.try_writing_segment();
    // if some peer enters the waiting state
    ps.if_waiting([this](int assigned_peer_id) {
        Message m(MessageType::GET_PICTURE_SEGMENT);
        // it is enough
        if (ps.all_segments_asked()) {
            return;
        }
        GetPictureSegment gps;
        gps.segment_id = ps.get_next_segment_id();
        m << gps;
        push_message(ps.get_peer_id(assigned_peer_id), m);
    });
}

void Client::start_file_sharing() {
    ps.reset_sharing_file();
    ps.open_file_for_writing();
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
