#ifndef CLIENT_H
#define CLIENT_H

#include "base-client.h"
#include "chunked-file.h"
#include "file-sharing.h"
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
    void on_connect(peer_id id) override;
    void on_disconnect(peer_id id) override;

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
    void handle_get_database(MessageWithOwner &t);
    void handle_return_database(MessageWithOwner &t);

    void cycle();
    void additional_cycle_hook();
    void start_file_sharing(const std::string &filename);
    FileSharing fs;

  private:
    Store s;
    int assigned_peer_id = 0;
    ChunkedFile cf;
    std::ofstream os;
};

#endif
