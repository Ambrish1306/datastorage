#include "core/Config.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace datastorage {

namespace {

std::string trimCopy(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r')) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n')) {
        --end;
    }

    return value.substr(start, end - start);
}

}  // namespace

Config::Config() = default;

Config::Config(const std::string& clusterPath) {
    loadFromFile(clusterPath);
}

const ClusterConfig& Config::cluster() const noexcept {
    return clusterConfig_;
}

std::string Config::trim(const std::string& value) {
    return trimCopy(value);
}

int Config::parsePositiveInt(const std::string& value, const std::string& name) {
    const std::string trimmed = trim(value);
    if (trimmed.empty()) {
        throw std::runtime_error("Missing value for " + name);
    }

    try {
        const int parsed = std::stoi(trimmed);
        if (parsed <= 0) {
            throw std::runtime_error(name + " must be greater than zero");
        }
        return parsed;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid integer value for " + name + ": " + trimmed);
    }
}

void Config::loadFromFile(const std::string& clusterPath) {
    std::ifstream input(clusterPath.c_str());
    if (!input) {
        throw std::runtime_error("Failed to open cluster config: " + clusterPath);
    }

    ClusterConfig config;
    std::string line;
    std::string currentSection;
    std::vector<NodeConfig> nodes;

    while (std::getline(input, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trim(trimmed.substr(1, trimmed.size() - 2));
            continue;
        }

        const std::size_t equalsPos = trimmed.find('=');
        if (equalsPos == std::string::npos) {
            throw std::runtime_error("Malformed line in cluster config: " + line);
        }

        const std::string key = trim(trimmed.substr(0, equalsPos));
        const std::string value = trim(trimmed.substr(equalsPos + 1));

        if (currentSection == "cluster") {
            if (key == "node_count") {
                config.nodeCount = parsePositiveInt(value, "node_count");
            } else {
                throw std::runtime_error("Unknown cluster option: " + key);
            }
        } else if (currentSection.find("node.") == 0) {
            const std::string indexText = currentSection.substr(5);
            if (indexText.empty()) {
                throw std::runtime_error("Invalid node section name: " + currentSection);
            }

            const int index = parsePositiveInt(indexText, "node index");
            if (static_cast<int>(nodes.size()) < index) {
                nodes.resize(static_cast<std::size_t>(index));
            }

            if (nodes.size() < static_cast<std::size_t>(index)) {
                throw std::runtime_error("Node section indices must be contiguous: " + currentSection);
            }

            if (nodes[static_cast<std::size_t>(index - 1)].id == 0 && nodes[static_cast<std::size_t>(index - 1)].inputFile.empty()) {
                nodes[static_cast<std::size_t>(index - 1)] = NodeConfig();
            }

            if (key == "id") {
                nodes[static_cast<std::size_t>(index - 1)].id = parsePositiveInt(value, "node id");
            } else if (key == "input_file") {
                if (value.empty()) {
                    throw std::runtime_error("input_file is required for node " + std::to_string(index));
                }
                nodes[static_cast<std::size_t>(index - 1)].inputFile = value;
            } else {
                throw std::runtime_error("Unknown option for node " + std::to_string(index) + ": " + key);
            }
        }
    }

    // Enforce architectural limits: cluster size must be between 1 and 5 nodes
    if (config.nodeCount <= 0) {
        throw std::runtime_error("Invalid cluster configuration: node_count must be positive");
    }
    
    if (config.nodeCount > 5) {
        throw std::runtime_error("Invalid cluster configuration: node_count must not exceed 5 (architectural limit)");
    }

    if (!nodes.empty()) {
        config.nodes = nodes;
    }

    if (config.nodes.size() != static_cast<std::size_t>(config.nodeCount)) {
        throw std::runtime_error("Cluster configuration mismatch: expected " + std::to_string(config.nodeCount) +
                                 " nodes but found " + std::to_string(config.nodes.size()));
    }

    for (std::size_t i = 0; i < config.nodes.size(); ++i) {
        const NodeConfig& node = config.nodes[i];
        if (node.id <= 0) {
            throw std::runtime_error("Node definition at index " + std::to_string(i + 1) + " is missing a valid id");
        }
        if (node.inputFile.empty()) {
            throw std::runtime_error("Node " + std::to_string(node.id) + " is missing input_file");
        }
    }

    for (std::size_t i = 0; i < config.nodes.size(); ++i) {
        for (std::size_t j = i + 1; j < config.nodes.size(); ++j) {
            if (config.nodes[i].id == config.nodes[j].id) {
                throw std::runtime_error("Duplicate node id detected: " + std::to_string(config.nodes[i].id));
            }
        }
    }

    clusterConfig_ = config;
}

}  // namespace datastorage
