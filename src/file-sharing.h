#ifndef PICTURE_SHARING_H
#define PICTURE_SHARING_H

#include "message-type.h"
#include <asio.hpp>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

using namespace std::literals;

class FileSharing {
  public:
    FileSharing();
    ~FileSharing();
    void reset_sharing_file();

    // see if there is any segment in the queue, if there is
    // call write_segment are written
    void try_writing_segment(
        std::function<void(const ReturnSegment &, bool)> write_segment);
    int get_next_assigned_id();

    void push_segment(ReturnSegment rps);
    int get_next_segment_id();
    bool all_segments_asked();
    int get_segment_count();
    void set_segment_count(int t);
    int new_peer(peer_id id);
    // give the peer 10 seconds to send the data back
    void start_timeout(int assigned_id);
    // if nothing, drop it
    void end_timeout(int assigned_id);
    // if the waiting flag is on for a peer, execute the handler
    void if_idle(std::function<void(int)> handler);
    int get_peer_id(int assigned_id);

    void increment_peer_failure(int assigned_id);
    void set_peer_idle(int assigned_id);
    void unset_peer_idle(int assigned_id);
    void die_peer(int assigned_id);
    bool is_peer_dead(int assigned_id);
    bool is_peer_idle(int assigned_id);

    void pause_writing();
    void resume_writing();
    bool paused();
    void set_file_info(int byte_per_chunk, int total_bytes);
    void should_pause();
    void must_pause();
    void stop_must_pause();

  private:
    std::vector<std::queue<ReturnSegment>> queue_buffer;
    int current_segment_id = -1;
    int current_byte = 0;
    int current_writing_id = 0;
    std::ofstream os;
    int current_assigned_id = 0;
    int total_segment_count;
    bool pause = false;
    bool hard_pause = false;

    void write_segment(const ReturnSegment &rps);

    uint8_t increment_failure(uint8_t state);
    uint8_t set_idle(uint8_t state);
    uint8_t unset_idle(uint8_t state);
    uint8_t die(uint8_t state);
    bool is_dead(uint8_t state);
    bool is_idle(uint8_t state);
    bool is_in_range(int assigned_id);

    // this flag will be for peer if it is idling (the queue is full) or
    // the there is no response from the peer
    std::vector<uint8_t> status;
    std::vector<asio::high_resolution_timer> timeout_timers;
    asio::io_context ctx;
    std::thread worker;
    std::vector<int> peer_map;
    int total_bytes;
    int queue_current_bytes;
    int bytes_per_chunk;
};

#endif
