#ifndef APPLICATION_CLIENT_H
#define APPLICATION_CLIENT_H

#include "base-client.h"
#include "message-type.h"
#include "message.h"

class ApplicationClient : public BaseClient {
  public:
    // handle_message will call the handler function
    // handler receives a MessageWithOwner object
    // which can send another message etc.
    ApplicationClient(uint16_t port,
                      std::function<void(MessageWithOwner &)> handler);
    ~ApplicationClient();

    void handle_message(MessageWithOwner &msg) override;

    void cycle();

  private:
    std::function<void(MessageWithOwner &)> handler;
};

#endif
