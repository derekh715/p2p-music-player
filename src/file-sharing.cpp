#include "file-sharing.h"
#include <asio/prefer.hpp>

FileSharing::FileSharing() {
    worker = std::thread([this]() { ctx.run(); });
}

FileSharing::~FileSharing() {
    if (worker.joinable()) {
        worker.join();
    }
}

void FileSharing::write_segment(const ReturnSegment &rps) {}

void FileSharing::try_writing_segment(
    std::function<void(const ReturnSegment &, bool)> write_segment) {
    if (block_write) {
        return;
    }
    bool all_empty = true;
    // try to write two segments if possible
    for (int i = 0; i < 2; i++) {
        // traverse through the list to see if there are in-order segments
        // if yes write it to the output stream
        for (auto &q : queue_buffer) {
            if (q.empty()) {
                continue;
            }
            all_empty = false;
            auto &rps = q.front();
            if (rps.segment_id == current_writing_id) {
                std::cout << "Writing " << rps.body.size()
                          << " bytes to the file (" << current_byte << " to "
                          << current_byte + rps.body.size() << ")"
                          << " segment: " << rps.segment_id << "/"
                          << total_segment_count - 1 << std::endl;
                current_byte += rps.body.size();
                bool end = ++current_writing_id >= total_segment_count;
                write_segment(rps, end);
                q.pop();
                // all requests needed are made, exit now
                if (end) {
                    return;
                }
            } else if (rps.segment_id < current_writing_id) {
                // if current_writing_id is over already, drop that segment
                // without writing
                q.pop();
            }
        }
        if (all_empty) {
            break;
        }
        all_empty = true;
    }
}

int FileSharing::get_next_assigned_id() { return ++current_assigned_id; }

void FileSharing::reset_sharing_file() {
    current_segment_id = -1;
    total_segment_count = 0;
    current_byte = 0;
    current_assigned_id = -1;
    peer_map.clear();
    waiting.clear();
    timeout_timers.clear();
    // open_file_for_writing();
}

void FileSharing::push_segment(ReturnSegment rps) {
    queue_buffer[rps.assigned_id_for_peer].push(rps);
}
int FileSharing::get_next_segment_id() { return ++current_segment_id; }

bool FileSharing::all_segments_asked() {
    return current_segment_id >= total_segment_count - 1;
}

int FileSharing::get_segment_count() { return total_segment_count; }

void FileSharing::set_segment_count(int t) { total_segment_count = t; }

int FileSharing::new_peer(peer_id id) {
    std::cout << "[PS] new peer " << id << std::endl;
    queue_buffer.push_back(std::queue<ReturnSegment>());
    waiting.push_back(false);
    timeout_timers.push_back(asio::high_resolution_timer(ctx));
    peer_map.push_back(id);
    return ++current_assigned_id;
}

void FileSharing::start_timeout(int assigned_id) {
    // try to wait for 10s
    // if nothing is returned set the waiting bit to be true
    timeout_timers[assigned_id].expires_from_now(10s);
    timeout_timers[assigned_id].async_wait([&](asio::error_code ec) {
        waiting[assigned_id] = true;
        // don't have this segment, it is over
        current_writing_id++;
    });
}
void FileSharing::end_timeout(int assigned_id) {
    timeout_timers[assigned_id].cancel();
}

void FileSharing::if_waiting(std::function<void(int)> handler) {
    // std::cout << "[PS] checking wait " << std::endl;
    for (int i = 0; i < waiting.size(); i++) {
        if (waiting[i]) {
            handler(i);
        }
    }
}

int FileSharing::get_peer_id(int assigned_id) { return peer_map[assigned_id]; }

void FileSharing::pause_writing() { block_write = true; }

void FileSharing::resume_writing() { block_write = false; }
