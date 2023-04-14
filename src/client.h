#ifndef CLIENT_H
#define CLIENT_H

#include "base-client.h"
#include "store.h"

class Client: public BaseClient {
public:
    Client(uint16_t port, const std::string &filename);

    void handle_message(MessageWithOwner &msg) override;

    void populate_tracks();

private:
    Store s;
};

#endif
