#include "graph-builder.h"

#include "evacuation-building.h"
#include "ns3/simulator.h"

#include <set>

std::vector<GraphFeature>
GraphBuilder::BuildGraphFeatures(
    const ns3::NodeContainer& userNodes)
{
    std::vector<GraphFeature> features;

    std::set<uint32_t> activeUsers =
        ns3::simple::MyBuilding::GetActiveUsers();


    for (uint32_t userIndex : activeUsers)
    {
        if (userIndex >= userNodes.GetN())
        {
            continue;
        }

        ns3::Ptr<ns3::Node> userNode =
            userNodes.Get(userIndex);

        ns3::Ptr<ns3::simple::MyUser> user =
            ns3::DynamicCast<ns3::simple::MyUser>(userNode);

        if (user == nullptr)
        {
            continue;
        }

        GraphFeature feature =
            CreateGraphFeature(userIndex, user);

        features.push_back(feature);
    }

    
    // ===== ここからデバッグ =====
    std::cout
        << "[GraphFeatures]"
        << " time=" << ns3::Simulator::Now().GetSeconds()
        << ", size=" << features.size()
        << std::endl;

    for (const auto& feature : features)
    {
        std::cout
            << "userId=" << feature.userId
            << ", x=" << feature.x
            << ", y=" << feature.y
            << ", speed=" << feature.speed
            << ", exitNode=" << feature.exitNode
            << std::endl;
    }
    // ===== ここまで =====
    

    return features;
}