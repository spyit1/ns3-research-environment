#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"

#include <iostream>
#include <fstream>
#include <cmath>

using namespace ns3;

// ============================================================
// 設定値
// ============================================================

// ノード数
const uint32_t NUM_NODES = 6;

// シミュレーション時間 [秒]
const double SIMULATION_TIME = 200.0;

// 位置を確認する間隔 [秒]
const double OUTPUT_INTERVAL = 10.0;

const std::string CLUSTER_VIEW_FILE =
    "graphsage_test_view.txt";


// GraphSAGE学習用特徴量の取得間隔 [秒]
const double FEATURE_INTERVAL = 5.0;

// GraphSAGE学習用ログ
const std::string TRAINING_LOG_FILE =
    "graphsage_training.log";

// 1ホップの近隣判定距離 [m]
const double NEIGHBOR_DISTANCE = 100.0;

// GraphSAGE用Edgeログ
const std::string EDGE_LOG_FILE =
    "graphsage_edge.log";



void
WriteClusterViewLog(NodeContainer nodes)
{
    double now = Simulator::Now().GetSeconds();

    std::ofstream fout;
    fout.open(CLUSTER_VIEW_FILE, std::ios::app);

    if (!fout)
    {
        std::cerr << "Failed to open ClusterViewer log file."
                  << std::endl;
        return;
    }

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mobility =
            nodes.Get(i)->GetObject<MobilityModel>();

        Vector position = mobility->GetPosition();

        // ClusterViewer用 NIログ
        fout << "NI, "
             << now << ", "
             << i + 1 << ", "
             << position.x << ", "
             << position.y << ", "
             << "0, "
             << "100, "
             << "0, "
             << "1"
             << std::endl;
    }

    fout.close();

    if (now + 1.0 < SIMULATION_TIME)
    {
        Simulator::Schedule(
            Seconds(1.0),
            &WriteClusterViewLog,
            nodes);
    }
}

void
WriteTrainingLog(NodeContainer nodes)
{
    double now = Simulator::Now().GetSeconds();

    std::ofstream fout;
    fout.open(TRAINING_LOG_FILE, std::ios::app);

    if (!fout)
    {
        std::cerr << "Failed to open training log file."
                  << std::endl;
        return;
    }

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mobility =
            nodes.Get(i)->GetObject<MobilityModel>();

        Vector position = mobility->GetPosition();

        fout << now << " "
             << i << " "
             << position.x << " "
             << position.y
             << std::endl;
    }

    fout.close();

    // 次の特徴量取得
    if (now + FEATURE_INTERVAL < SIMULATION_TIME)
    {
        Simulator::Schedule(
            Seconds(FEATURE_INTERVAL),
            &WriteTrainingLog,
            nodes);
    }
}

void
WriteEdgeLog(NodeContainer nodes)
{
    double now = Simulator::Now().GetSeconds();

    std::ofstream fout;
    fout.open(EDGE_LOG_FILE, std::ios::app);

    if (!fout)
    {
        std::cerr << "Failed to open edge log file."
                  << std::endl;
        return;
    }

    // 全ノードの組み合わせを確認
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mobilityI =
            nodes.Get(i)->GetObject<MobilityModel>();

        Vector positionI =
            mobilityI->GetPosition();

        for (uint32_t j = i + 1; j < nodes.GetN(); ++j)
        {
            Ptr<MobilityModel> mobilityJ =
                nodes.Get(j)->GetObject<MobilityModel>();

            Vector positionJ =
                mobilityJ->GetPosition();

            double dx = positionI.x - positionJ.x;
            double dy = positionI.y - positionJ.y;

            double distance =
                std::sqrt(dx * dx + dy * dy);

            // 100m以内なら1ホップ近隣
            if (distance <= NEIGHBOR_DISTANCE)
            {
                fout << now << " "
                     << i << " "
                     << j
                     << std::endl;
            }
        }
    }

    fout.close();

    // 特徴量と同じ5秒間隔で更新
    if (now + FEATURE_INTERVAL < SIMULATION_TIME)
    {
        Simulator::Schedule(
            Seconds(FEATURE_INTERVAL),
            &WriteEdgeLog,
            nodes);
    }
}
    
// ============================================================
// 全ノードの現在位置を表示する関数
// ============================================================
void
PrintPositions(NodeContainer nodes)
{
    double now = Simulator::Now().GetSeconds();

    std::cout << "===== time = "
              << now
              << " s ====="
              << std::endl;

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Node> node = nodes.Get(i);

        Ptr<MobilityModel> mobility =
            node->GetObject<MobilityModel>();

        Vector position = mobility->GetPosition();

        std::cout
            << "userId=" << i
            << ", x=" << position.x
            << ", y=" << position.y
            << std::endl;
    }

    // 次の位置確認を予約
    if (now + OUTPUT_INTERVAL <= SIMULATION_TIME)
    {
        Simulator::Schedule(
            Seconds(OUTPUT_INTERVAL),
            &PrintPositions,
            nodes);
    }
}


// ============================================================
// main
// ============================================================
int
main(int argc, char* argv[])
{
    // --------------------------------------------------------
    // 1. ノードを6個作成
    // --------------------------------------------------------
    NodeContainer nodes;
    nodes.Create(NUM_NODES);

    // ClusterViewerログを初期化
    {
        std::ofstream fout(CLUSTER_VIEW_FILE);

        if (!fout)
        {
            std::cerr << "Failed to create ClusterViewer log file."
                    << std::endl;
            return 1;
        }

        // ノード数, フィールド幅, フィールド高さ
        fout << NUM_NODES
            << ",1000,1000"
            << std::endl;
    }


    // GraphSAGE学習用ログを初期化
    {
        std::ofstream fout(TRAINING_LOG_FILE);

        if (!fout)
        {
            std::cerr << "Failed to create training log file."
                    << std::endl;
            return 1;
        }
    }


    // GraphSAGE Edgeログを初期化
    {
        std::ofstream fout(EDGE_LOG_FILE);

        if (!fout)
        {
            std::cerr << "Failed to create edge log file."
                    << std::endl;
            return 1;
        }
    }


    // --------------------------------------------------------
    // 2. WaypointMobilityModelを設定
    // --------------------------------------------------------
    MobilityHelper mobility;

    mobility.SetMobilityModel(
        "ns3::WaypointMobilityModel",
        "InitialPositionIsWaypoint",
        BooleanValue(false));

    mobility.Install(nodes);


    // --------------------------------------------------------
    // 3. 各ノードにWaypointを設定
    // --------------------------------------------------------

    // Group A
    //
    // Node 0 : (100,100) → (500,100)
    // Node 1 : (100,120) → (500,120)
    // Node 2 : (100,140) → (500,140)
    //
    // 3ノードとも右方向へ移動

    for (uint32_t i = 0; i < 3; ++i)
    {
        Ptr<WaypointMobilityModel> mob =
            nodes.Get(i)->GetObject<WaypointMobilityModel>();

        double y = 100.0 + 20.0 * i;

        // 0秒時点
        mob->AddWaypoint(
            Waypoint(
                Seconds(0.0),
                Vector(100.0, y, 0.0)));

        // 200秒時点
        mob->AddWaypoint(
            Waypoint(
                Seconds(SIMULATION_TIME),
                Vector(500.0, y, 0.0)));
    }


    // Group B
    //
    // Node 3 : (700,500) → (700,900)
    // Node 4 : (720,500) → (720,900)
    // Node 5 : (740,500) → (740,900)
    //
    // 3ノードとも上方向へ移動

    for (uint32_t i = 3; i < 6; ++i)
    {
        Ptr<WaypointMobilityModel> mob =
            nodes.Get(i)->GetObject<WaypointMobilityModel>();

        double x = 700.0 + 20.0 * (i - 3);

        // 0秒時点
        mob->AddWaypoint(
            Waypoint(
                Seconds(0.0),
                Vector(x, 500.0, 0.0)));

        // 200秒時点
        mob->AddWaypoint(
            Waypoint(
                Seconds(SIMULATION_TIME),
                Vector(x, 900.0, 0.0)));
    }


    // --------------------------------------------------------
    // 4. 位置表示を開始
    // --------------------------------------------------------
    Simulator::Schedule(
        Seconds(0.0),
        &PrintPositions,
        nodes);

    Simulator::Schedule(
        Seconds(0.0),
        &WriteClusterViewLog,
        nodes);

    Simulator::Schedule(
        Seconds(0.0),
        &WriteTrainingLog,
        nodes);

    Simulator::Schedule(
        Seconds(0.0),
        &WriteEdgeLog,
        nodes);

    // --------------------------------------------------------
    // 5. シミュレーション実行
    // --------------------------------------------------------
    Simulator::Stop(
        Seconds(SIMULATION_TIME));

    Simulator::Run();

    Simulator::Destroy();

    return 0;
}

