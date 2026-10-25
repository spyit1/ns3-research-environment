#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

using namespace ns3;

// ============================================================
// 設定値
// ============================================================

// ノード数
const uint32_t NUM_NODES = 50;

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


// ============================================================
// 各ノードの目的地
// ============================================================

std::vector<Vector> nodeDestinations;    



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
        Vector velocity = mobility->GetVelocity();

        // ----------------------------------------------------
        // 各ノードの目的地
        // ----------------------------------------------------
        double destinationX = nodeDestinations[i].x;
        double destinationY = nodeDestinations[i].y;


        fout << now << " "
            << i << " "
            << position.x << " "
            << position.y << " "
            << velocity.x << " "
            << velocity.y << " "
            << destinationX << " "
            << destinationY
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
    uint32_t seed = 1;
    uint64_t run = 1;

    CommandLine cmd;

    cmd.AddValue(
        "seed",
        "Random seed",
        seed);

    cmd.AddValue(
        "run",
        "Random run number",
        run);

    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(run);

    // --------------------------------------------------------
    // 1. ノードを8個作成
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
    // 3. 各ノードにランダムWaypointを設定
    // --------------------------------------------------------

    // ============================================================
    // 50ノードの初期位置をランダムに設定
    // ============================================================

    std::vector<Vector> initialPositions(NUM_NODES);

    // 初期位置用乱数生成器
    Ptr<UniformRandomVariable> randomInitialPosition =
        CreateObject<UniformRandomVariable>();

    randomInitialPosition->SetAttribute(
        "Min",
        DoubleValue(100.0));

    randomInitialPosition->SetAttribute(
        "Max",
        DoubleValue(900.0));

    // 全ノードの初期位置を生成
    for (uint32_t i = 0; i < NUM_NODES; ++i)
    {
        double startX = randomInitialPosition->GetValue();
        double startY = randomInitialPosition->GetValue();

        initialPositions[i] = Vector(
            startX,
            startY,
            0.0
        );
    }

    // 目的地を保存する領域
    nodeDestinations.resize(NUM_NODES);

    // 乱数生成器
    Ptr<UniformRandomVariable> randomPosition =
        CreateObject<UniformRandomVariable>();

    Ptr<UniformRandomVariable> randomSpeed =
        CreateObject<UniformRandomVariable>();

    randomPosition->SetAttribute(
        "Min",
        DoubleValue(100.0));

    randomPosition->SetAttribute(
        "Max",
        DoubleValue(900.0));

    randomSpeed->SetAttribute(
        "Min",
        DoubleValue(1.0));

    randomSpeed->SetAttribute(
        "Max",
        DoubleValue(5.0));

    // 各ノードのWaypointを設定
    for (uint32_t i = 0; i < NUM_NODES; ++i)
    {
        Ptr<WaypointMobilityModel> mob =
            nodes.Get(i)->GetObject<WaypointMobilityModel>();

        Vector start = initialPositions[i];

        // ランダムな目的地
        double destX = randomPosition->GetValue();
        double destY = randomPosition->GetValue();

        Vector destination(destX, destY, 0.0);

        nodeDestinations[i] = destination;

        // ランダムな移動速度
        double speed = randomSpeed->GetValue();

        // 移動距離
        double dx = destination.x - start.x;
        double dy = destination.y - start.y;

        double distance =
            std::sqrt(dx * dx + dy * dy);

        // 目的地までの移動時間
        double travelTime = distance / speed;

        // 初期位置
        mob->AddWaypoint(
            Waypoint(
                Seconds(0.0),
                start));

        // 目的地
        mob->AddWaypoint(
            Waypoint(
                Seconds(travelTime),
                destination));

        std::cout
            << "Node " << i
            << " Start=(" << start.x << "," << start.y << ")"
            << " Destination=(" << destX << "," << destY << ")"
            << " Speed=" << speed
            << " m/s"
            << " ArrivalTime=" << travelTime
            << " s"
            << std::endl;
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

