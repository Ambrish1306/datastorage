#include "network/Transport.h"

#include <stdexcept>

namespace datastorage {

std::vector<std::string> MockSocketTransport::globalMessages_;
std::vector<std::int32_t> MockSocketTransport::globalDestinations_;

MockSocketTransport::MockSocketTransport()
    : nodeId_(0),
      connected_(false),
      inbox_() {
}

MockSocketTransport::MockSocketTransport(std::int32_t nodeId)
    : nodeId_(nodeId),
      connected_(false),
      inbox_() {
}

void MockSocketTransport::connect(std::int32_t nodeId) {
    nodeId_ = nodeId;
    connected_ = true;
}

void MockSocketTransport::send(const std::string& payload) {
    if (!connected_) {
        throw std::runtime_error("Transport is not connected");
    }
    if (payload.empty()) {
        throw std::runtime_error("Cannot send an empty payload");
    }

    globalMessages_.push_back(payload);
    globalDestinations_.push_back(nodeId_);
}

std::string MockSocketTransport::recv() {
    if (!connected_) {
        throw std::runtime_error("Transport is not connected");
    }
    if (inbox_.empty()) {
        return std::string();
    }

    std::string message = inbox_.front();
    inbox_.erase(inbox_.begin());
    return message;
}

void MockSocketTransport::close() {
    connected_ = false;
    inbox_.clear();
}

std::int32_t MockSocketTransport::nodeId() const {
    return nodeId_;
}

std::size_t MockSocketTransport::queuedMessages() const {
    return inbox_.size();
}

}  // namespace datastorage
