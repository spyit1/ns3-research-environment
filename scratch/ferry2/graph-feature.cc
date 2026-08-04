#include "graph-feature.h"

#include "ns3/mobility-model.h"
#include "ns3/vector.h"

#include <string>
#include <cmath>

GraphFeature
CreateGraphFeature(
    uint32_t userId,
    ns3::Ptr<ns3::simple::MyUser> user)
{
    GraphFeature feature;

    feature.userId = userId;
    feature.x = 0.0;
    feature.y = 0.0;

    feature.speed = 0.0;
    feature.exitNode = "";

    if (user == nullptr)
    {
        return feature;
    }

    ns3::Ptr<ns3::MobilityModel> mobility =
        user->GetObject<ns3::MobilityModel>();

    if (mobility == nullptr)
    {
        return feature;
    }

    ns3::Vector position = mobility->GetPosition();

    feature.x = position.x;
    feature.y = position.y;

    ns3::Vector velocity = mobility->GetVelocity();

    feature.speed = std::sqrt(
        velocity.x * velocity.x +
        velocity.y * velocity.y);
    
    feature.exitNode = user->GetExitnode();

    return feature;
}