#ifndef DATASTORAGE_CONFIG_H
#define DATASTORAGE_CONFIG_H

#include <string>
#include <vector>

namespace datastorage {

struct NodeConfig {
    int id = 0;
    std::string inputFile;
};

struct ClusterConfig {
    int nodeCount = 0;
    std::vector<NodeConfig> nodes;
};
//ClusterConfig cluster; cluster.nodes.push_back({1, "node1.txt"});

class Config {
public:
    Config();
    explicit Config(const std::string& clusterPath);

    const ClusterConfig& cluster() const noexcept;
    void loadFromFile(const std::string& clusterPath);

private:
    static std::string trim(const std::string& value);
    static int parsePositiveInt(const std::string& value, const std::string& name);

    ClusterConfig clusterConfig_;
};

}  // namespace datastorage

#endif  // DATASTORAGE_CONFIG_H
