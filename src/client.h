#ifndef CLIENT_H
#define CLIENT_H

#include "base-client.h"
#include "chunked-file.h"
#include "store.h"
#include <fstream>
#include <iostream>
#include <queue>

class Client : public BaseClient {
  public:
    Client(uint16_t port, const std::string &filename);
    ~Client();

    void handle_message(MessageWithOwner &msg) override;

    void populate_tracks();

    // handlers
    void handle_ping(MessageWithOwner &t);
    void handle_pong(MessageWithOwner &t);
    void handle_no_such_track(MessageWithOwner &t);
    void handle_return_track_info(MessageWithOwner &t);
    void handle_get_track_info(MessageWithOwner &t);
    void handle_no_such_lyrics(MessageWithOwner &t);
    void handle_get_lyrics(MessageWithOwner &t);
    void handle_return_lyrics(MessageWithOwner &t);
    void handle_has_picture_file(MessageWithOwner &t);
    void handle_prepare_picture_sharing(MessageWithOwner &t);
    void handle_prepared_picture_sharing(MessageWithOwner &t);
    void handle_return_picture_segment(MessageWithOwner &t);
    void handle_get_picture_segment(MessageWithOwner &t);

    std::map<peer_id, std::shared_ptr<tcp::socket>> get_peers();
    void open_file_for_writing();
    void reset_sharing_file();
    void try_writing_segment();
    void write_segment(const ReturnPictureSegment &rps);
    void additional_cycle_hook() override;

    int current_assigned_id = 0;

  private:
    Store s;
    int current_segment_id = 0;
    int total_segment_count = 0;
    int current_byte = 0;
    int current_writing_id = 0;
    int assigned_peer_id = 0;
    std::vector<std::queue<ReturnPictureSegment>> queue_buffer;

    ChunkedFile cf;
    std::ofstream os;
};

#endif
