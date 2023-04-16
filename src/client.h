#ifndef CLIENT_H
#define CLIENT_H

#include "base-client.h"
#include "store.h"

class Client : public BaseClient {
  public:
    Client(uint16_t port, const std::string &filename);

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

  private:
    Store s;
};

#endif
