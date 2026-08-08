#ifndef DATASTORAGE_TRANSPORT_H
#define DATASTORAGE_TRANSPORT_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace datastorage {

class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    virtual void connect(std::int32_t nodeId) = 0;
    virtual void send(const std::string& payload) = 0;
    virtual std::string recv() = 0;
    virtual void close() = 0;
};

class MockSocketTransport : public INetworkTransport {
public:
    MockSocketTransport();
    explicit MockSocketTransport(std::int32_t nodeId);

    void connect(std::int32_t nodeId) override;
    void send(const std::string& payload) override;
    std::string recv() override;
    void close() override;

    std::int32_t nodeId() const;
    std::size_t queuedMessages() const;

private:
    std::int32_t nodeId_;
    bool connected_;
    std::vector<std::string> inbox_;
    static std::vector<std::string> globalMessages_;
    static std::vector<std::int32_t> globalDestinations_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_TRANSPORT_H
