#include "file-sharing.h"

FileSharing::FileSharing() {
    pause = false;
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
    if (pause || hard_pause) {
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
            // std::cout << "RPS: " << rps.segment_id
            //           << " | CWI: " << current_writing_id << std::endl;
            if (rps.segment_id == current_writing_id) {
                std::cout << "Writing " << rps.body.size()
                          << " bytes to the file (" << current_byte << " to "
                          << current_byte + rps.body.size() << ")"
                          << " segment: " << rps.segment_id << "/"
                          << total_segment_count - 1 << std::endl;
                current_byte += rps.body.size();
                bool end = ++current_writing_id >= total_segment_count;
                write_segment(rps, end);
                queue_current_bytes -= bytes_per_chunk;
                q.pop();
                // all requests needed are made, exit now
                if (end) {
                    return;
                }
            } else if (rps.segment_id < current_writing_id) {
                // if current_writing_id is over already, drop that segment
                // without writing
                q.pop();
            } else if (rps.segment_id > current_writing_id + status.size()) {
                // also pop very high segment numbers since they block the
                // queue!
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
    current_assigned_id = 0;
    current_writing_id = 0;
    peer_map.clear();
    status.clear();
    timeout_timers.clear();
    pause = false;
    bytes_per_chunk = 0;
    queue_current_bytes = 0;
    total_bytes = 0;
    hard_pause = false;
    // also drop all the previous buffers
    queue_buffer = std::vector<std::queue<ReturnSegment>>();
    // open_file_for_writing();
}

bool FileSharing::paused() { return pause; }

void FileSharing::push_segment(ReturnSegment rps) {
    if (!is_in_range(rps.assigned_id_for_peer)) {
        return;
    }
    queue_current_bytes += bytes_per_chunk;
    queue_buffer[rps.assigned_id_for_peer].push(rps);
}
int FileSharing::get_next_segment_id() { return ++current_segment_id; }

bool FileSharing::all_segments_asked() {
    return current_segment_id >= total_segment_count - 1;
}

int FileSharing::get_segment_count() { return total_segment_count; }

void FileSharing::set_segment_count(int t) { total_segment_count = t; }

int FileSharing::new_peer(peer_id id) {
    queue_buffer.push_back(std::queue<ReturnSegment>());
    status.push_back(0);
    timeout_timers.push_back(asio::high_resolution_timer(ctx));
    peer_map.push_back(id);
    return current_assigned_id++;
}

void FileSharing::start_timeout(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    // try to wait for 10s
    // if nothing is returned set the waiting bit to be true
    timeout_timers[assigned_id].expires_from_now(10s);
    timeout_timers[assigned_id].async_wait([&](asio::error_code ec) {
        status[assigned_id] = increment_failure(status[assigned_id]);
        // don't have this segment, it is over
        current_writing_id++;
    });
}
void FileSharing::end_timeout(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    timeout_timers[assigned_id].cancel();
}

void FileSharing::if_idle(std::function<void(int)> handler) {
    // don't do anything if paused
    if (pause || hard_pause) {
        return;
    }
    for (int i = 0; i < status.size(); i++) {
        // don't ask dead peers and those that are busy
        if (!is_peer_dead(i) && is_peer_idle(i)) {
            handler(i);
            unset_peer_idle(i);
        }
    }
}

int FileSharing::get_peer_id(int assigned_id) { return peer_map[assigned_id]; }

void FileSharing::pause_writing() {
    pause = true;
    for (int i = 0; i < status.size(); i++) {
        timeout_timers[i].cancel();
        set_peer_idle(i);
    }
}

void FileSharing::must_pause() {
    // no matter what you must pause sharing!!
    hard_pause = true;
    for (int i = 0; i < status.size(); i++) {
        timeout_timers[i].cancel();
        set_peer_idle(i);
    }
}

void FileSharing::stop_must_pause() { hard_pause = false; }

void FileSharing::resume_writing() { pause = false; }

uint8_t FileSharing::increment_failure(uint8_t state) {
    uint8_t original_failure = state & 0b11;
    // already failed three times, declare it dead
    if (original_failure == 0b11) {
        return die(state);
    }
    return set_idle(state) | (original_failure + 1);
}

uint8_t FileSharing::set_idle(uint8_t state) { return state | 0b100; }

uint8_t FileSharing::unset_idle(uint8_t state) { return state & 0b11111011; }

uint8_t FileSharing::die(uint8_t state) { return state | 0b1000; }

bool FileSharing::is_dead(uint8_t state) { return (state & 0b1000) != 0; }
bool FileSharing::is_idle(uint8_t state) { return (state & 0b100) != 0; }

void FileSharing::increment_peer_failure(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    status[assigned_id] = increment_failure(status[assigned_id]);
}
void FileSharing::set_peer_idle(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    status[assigned_id] = set_idle(status[assigned_id]);
}
void FileSharing::unset_peer_idle(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    status[assigned_id] = unset_idle(status[assigned_id]);
}
void FileSharing::die_peer(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return;
    }
    status[assigned_id] = die(status[assigned_id]);
}

bool FileSharing::is_peer_dead(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return false;
    }
    return is_dead(status[assigned_id]);
}

bool FileSharing::is_peer_idle(int assigned_id) {
    if (!is_in_range(assigned_id)) {
        return false;
    }
    return is_idle(status[assigned_id]);
}

bool FileSharing::is_in_range(int assigned_id) {
    return assigned_id > -1 && assigned_id < current_assigned_id;
}

void FileSharing::should_pause() {
    if (total_bytes == 0 || hard_pause) {
        return;
    }
    // std::cout << "QCB: " << queue_current_bytes
    //           << " | TB: " << total_bytes * 0.5 << " ("
    //           << (double)queue_current_bytes / total_bytes * 100 << "%)"
    //           << std::endl;
    if ((double)queue_current_bytes > total_bytes * 0.5) {
        pause_writing();
    } else {
        resume_writing();
    }
}

void FileSharing::set_file_info(int b, int t) {
    bytes_per_chunk = b;
    total_bytes = t;
}
