#ifndef GRAPH_FEATURE_H
#define GRAPH_FEATURE_H

#include <cstdint>
#include <string>

#include "evacuation-user.h"

struct GraphFeature
{
    uint32_t userId;

    double x;
    double y;

    double speed;

    std::string exitNode;
};

// 指定した避難ユーザから1人分の特徴量を作成する
GraphFeature CreateGraphFeature(
    uint32_t userId,
    ns3::Ptr<ns3::simple::MyUser> user);

#endif // GRAPH_FEATURE_H