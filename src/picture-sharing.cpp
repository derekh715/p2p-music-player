#include "picture-sharing.h"
#include <asio/prefer.hpp>

PictureSharing::PictureSharing() {
    worker = std::thread([this]() { ctx.run(); });
}

PictureSharing::~PictureSharing() {
    if (worker.joinable()) {
        worker.join();
    }
}

void PictureSharing::write_segment(const ReturnPictureSegment &rps) {
    std::cout << "Writing " << rps.body.size() << " bytes to the file ("
              << current_byte << " to " << current_byte + rps.body.size() << ")"
              << " segment: " << rps.segment_id << "/"
              << total_segment_count - 1 << std::endl;
    current_byte += rps.body.size();
    os.write(rps.body.data(), rps.body.size());
}

void PictureSharing::try_writing_segment() {
    // try to write two segments if possible
    for (int i = 0; i < 1; i++) {
        // traverse through the list to see if there are in-order segments
        // if yes write it to the output stream
        for (auto &q : queue_buffer) {
            if (q.empty()) {
                continue;
            }
            auto &rps = q.front();
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
            } else if (rps.segment_id < current_writing_id) {
                // if current_writing_id is over already, drop that segment
                // without writing
                q.pop();
            }
        }
    }
}

int PictureSharing::get_next_assigned_id() { return ++current_assigned_id; }

void PictureSharing::reset_sharing_file() {
    current_segment_id = -1;
    total_segment_count = 0;
    current_byte = 0;
    current_assigned_id = -1;
    peer_map.clear();
    waiting.clear();
    timeout_timers.clear();
    open_file_for_writing();
}

void PictureSharing::open_file_for_writing() {
    os.open("./interleaved.bmp",
            std::ios::out | std::ios::binary | std::ios::trunc);
    if (!os.is_open()) {
        std::cout << "For some reason the file cannot be opened for writing!"
                  << std::endl;
    }
}

void PictureSharing::push_segment(ReturnPictureSegment rps) {
    queue_buffer[rps.assigned_id_for_peer].push(rps);
}
int PictureSharing::get_next_segment_id() { return ++current_segment_id; }

bool PictureSharing::all_segments_asked() {
    return current_segment_id >= total_segment_count - 1;
}

int PictureSharing::get_segment_count() { return total_segment_count; }

void PictureSharing::set_segment_count(int t) { total_segment_count = t; }

int PictureSharing::new_peer(peer_id id) {
    std::cout << "[PS] new peer " << id << std::endl;
    queue_buffer.push_back(std::queue<ReturnPictureSegment>());
    waiting.push_back(false);
    timeout_timers.push_back(asio::high_resolution_timer(ctx));
    peer_map.push_back(id);
    return ++current_assigned_id;
}

void PictureSharing::start_timeout(int assigned_id) {
    std::cout << "[PS] start timeout " << assigned_id << std::endl;
    // try to wait for 10s
    // if nothing is returned set the waiting bit to be true
    timeout_timers[assigned_id].expires_from_now(10s);
    timeout_timers[assigned_id].async_wait([&](asio::error_code ec) {
        waiting[assigned_id] = true;
        // don't have this segment, it is over
        current_writing_id++;
    });
}
void PictureSharing::end_timeout(int assigned_id) {
    std::cout << "[PS] end timeout " << assigned_id << std::endl;
    timeout_timers[assigned_id].cancel();
}

void PictureSharing::if_waiting(std::function<void(int)> handler) {
    // std::cout << "[PS] checking wait " << std::endl;
    for (int i = 0; i < waiting.size(); i++) {
        if (waiting[i]) {
            handler(i);
        }
    }
}

int PictureSharing::get_peer_id(int assigned_id) {
    return peer_map[assigned_id];
}
