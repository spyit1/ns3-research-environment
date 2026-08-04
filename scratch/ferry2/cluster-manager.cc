#include "cluster-manager.h"

#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>

ClusterManager::ClusterManager()
{
}

void
ClusterManager::AssignUserToCluster(uint32_t userId, int32_t clusterId)
{
    auto userIt = m_userClusterMap.find(userId);

    if (userIt != m_userClusterMap.end())
    {
        int32_t oldClusterId = userIt->second;

        if (oldClusterId == clusterId)
        {
            return;
        }

        std::vector<uint32_t>& oldMembers =
            m_clusterMembersMap[oldClusterId];

        oldMembers.erase(
            std::remove(
                oldMembers.begin(),
                oldMembers.end(),
                userId),
            oldMembers.end());

        if (oldMembers.empty())
        {
            m_clusterMembersMap.erase(oldClusterId);
        }
    }

    m_userClusterMap[userId] = clusterId;
    m_clusterMembersMap[clusterId].push_back(userId);
}

int32_t
ClusterManager::GetClusterId(uint32_t userId) const
{
    auto userIt = m_userClusterMap.find(userId);

    if (userIt == m_userClusterMap.end())
    {
        return -1;
    }

    return userIt->second;
}

bool
ClusterManager::HasUser(uint32_t userId) const
{
    return m_userClusterMap.find(userId) != m_userClusterMap.end();
}