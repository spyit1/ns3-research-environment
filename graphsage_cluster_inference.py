import math
import torch
import torch.nn.functional as F

from collections import defaultdict
from torch_geometric.nn import SAGEConv


# ============================================================
# 設定
# ============================================================

MODEL_FILE = "graphsage_50nodes_run5_500epochs.pth"

FEATURE_FILE = "graphsage_scenarios/50nodes/run_6/graphsage_training.log"
EDGE_FILE = "graphsage_scenarios/50nodes/run_6/graphsage_edge.log"

OUTPUT_FILE = "graphsage_run6_cluster.log"

POSITION_SCALE = 1000.0
VELOCITY_SCALE = 5.0

EMBEDDING_THRESHOLD = 1.0


# ============================================================
# GraphSAGEモデル
# ============================================================

class GraphSAGE(torch.nn.Module):

    def __init__(self):

        super().__init__()

        self.conv1 = SAGEConv(6, 16)
        self.conv2 = SAGEConv(16, 8)

    def forward(self, x, edge_index):

        x = self.conv1(x, edge_index)
        x = F.relu(x)
        x = self.conv2(x, edge_index)

        return x


# ============================================================
# 特徴量ログ読み込み
# ============================================================

features_by_time = defaultdict(dict)

with open(FEATURE_FILE, "r") as f:

    for line in f:

        parts = line.split()

        if len(parts) != 8:
            continue

        time = float(parts[0])
        user_id = int(parts[1])

        x = float(parts[2])
        y = float(parts[3])

        vx = float(parts[4])
        vy = float(parts[5])

        dest_x = float(parts[6])
        dest_y = float(parts[7])

        features_by_time[time][user_id] = [
            x / POSITION_SCALE,
            y / POSITION_SCALE,
            vx / VELOCITY_SCALE,
            vy / VELOCITY_SCALE,
            dest_x / POSITION_SCALE,
            dest_y / POSITION_SCALE
        ]


# ============================================================
# Edgeログ読み込み
# ============================================================

edges_by_time = defaultdict(list)

with open(EDGE_FILE, "r") as f:

    for line in f:

        parts = line.split()

        if len(parts) != 3:
            continue

        time = float(parts[0])
        source = int(parts[1])
        target = int(parts[2])

        edges_by_time[time].append((source, target))
        edges_by_time[time].append((target, source))


# ============================================================
# 学習済みモデル読み込み
# ============================================================

checkpoint = torch.load(
    MODEL_FILE,
    map_location="cpu",
    weights_only=True
)

model = GraphSAGE()

model.load_state_dict(
    checkpoint["model_state_dict"]
)

model.eval()

print("Loaded pretrained model.")
print("Completed epochs:", checkpoint["completed_epochs"])


# ============================================================
# クラスタリング関数
# ============================================================

def clustering_from_embeddings(embeddings):

    num_nodes = embeddings.size(0)

    adjacency = [[] for _ in range(num_nodes)]

    # Embedding距離から接続関係を作成
    for i in range(num_nodes):

        for j in range(i + 1, num_nodes):

            distance = torch.norm(
                embeddings[i] - embeddings[j]
            ).item()

            if distance < EMBEDDING_THRESHOLD:

                adjacency[i].append(j)
                adjacency[j].append(i)

    # 接続成分をクラスタとして抽出
    cluster_labels = [-1] * num_nodes

    cluster_id = 0

    for start in range(num_nodes):

        if cluster_labels[start] != -1:
            continue

        stack = [start]
        cluster_labels[start] = cluster_id

        while stack:

            current = stack.pop()

            for neighbor in adjacency[current]:

                if cluster_labels[neighbor] == -1:

                    cluster_labels[neighbor] = cluster_id
                    stack.append(neighbor)

        cluster_id += 1

    return cluster_labels, cluster_id


# ============================================================
# 全時刻で推論・クラスタリング
# ============================================================

times = sorted(features_by_time.keys())

with open(OUTPUT_FILE, "w") as fout:

    for current_time in times:

        current_features = features_by_time[current_time]

        user_ids = sorted(current_features.keys())

        user_to_index = {
            user_id: index
            for index, user_id in enumerate(user_ids)
        }

        # Node features
        x_tensor = torch.tensor(
            [
                current_features[user_id]
                for user_id in user_ids
            ],
            dtype=torch.float
        )

        # edge_index
        edge_list = []

        for source, target in edges_by_time.get(
            current_time, []
        ):

            if (
                source in user_to_index
                and target in user_to_index
            ):

                edge_list.append([
                    user_to_index[source],
                    user_to_index[target]
                ])

        if edge_list:

            edge_index = torch.tensor(
                edge_list,
                dtype=torch.long
            ).t().contiguous()

        else:

            edge_index = torch.empty(
                (2, 0),
                dtype=torch.long
            )

        # GraphSAGE推論
        with torch.no_grad():

            embeddings = model(
                x_tensor,
                edge_index
            )

        # クラスタリング
        cluster_labels, cluster_count = (
            clustering_from_embeddings(embeddings)
        )

        # ログ出力
        for index, user_id in enumerate(user_ids):

            fout.write(
                f"{current_time:g} "
                f"{user_id} "
                f"{cluster_labels[index]}\n"
            )

        print(
            f"Time={current_time:6.1f}s "
            f"Nodes={len(user_ids)} "
            f"Clusters={cluster_count}"
        )


print("\nClustering finished.")
print("Saved:", OUTPUT_FILE)