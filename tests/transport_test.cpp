#include "network/Transport.h"

#include <cassert>
#include <stdexcept>

int main() {
    datastorage::MockSocketTransport transport(1);

    bool disconnectedSend = false;
    try {
        transport.send("payload");
    } catch (const std::runtime_error&) {
        disconnectedSend = true;
    }
    assert(disconnectedSend);

    transport.connect(2);
    transport.send("hello");
    assert(transport.nodeId() == 2);

    datastorage::MockSocketTransport receiver(3);
    receiver.connect(3);

    assert(receiver.queuedMessages() == 0u);
    receiver.close();

    return 0;
}
