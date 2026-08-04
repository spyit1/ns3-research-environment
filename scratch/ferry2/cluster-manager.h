#ifndef CLUSTER_MANAGER_H
#define CLUSTER_MANAGER_H

#include <cstdint>
#include <map>
#include <vector>

class ClusterManager
{
public:
    ClusterManager();

private:
    std::map<uint32_t, int32_t> m_userClusterMap;
    std::map<int32_t, std::vector<uint32_t>> m_clusterMembersMap;
    std::map<int32_t, uint32_t> m_clusterHeadMap;
};

#endif // CLUSTER_MANAGER_H