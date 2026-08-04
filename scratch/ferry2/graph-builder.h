#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include <vector>

#include "ns3/node-container.h"

#include "graph-feature.h"

class GraphBuilder
{
public:
    // アクティブユーザ全員分の特徴量を作成する
    static std::vector<GraphFeature> BuildGraphFeatures(
        const ns3::NodeContainer& userNodes);
};

#endif // GRAPH_BUILDER_H