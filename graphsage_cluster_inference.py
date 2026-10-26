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
# 前時刻のクラスタIDとの対応付け
# ============================================================

def match_cluster_ids(user_ids, cluster_labels, previous_clusters, next_id):

    # 今回のクラスタごとの所属ノード
    current_groups = defaultdict(set)

    for user_id, cluster_id in zip(user_ids, cluster_labels):
        current_groups[cluster_id].add(user_id)

    # 前時刻のクラスタごとの所属ノード
    previous_groups = defaultdict(set)

    for user_id, cluster_id in previous_clusters.items():
        previous_groups[cluster_id].add(user_id)

    # 重複ノード数が多い組み合わせから優先して対応付ける
    candidates = []

    for current_id, current_members in current_groups.items():

        for previous_id, previous_members in previous_groups.items():

            overlap = len(current_members & previous_members)

            if overlap > 0:
                candidates.append(
                    (overlap, current_id, previous_id)
                )

    candidates.sort(
        key=lambda item: (-item[0], item[2], item[1])
    )

    id_mapping = {}

    used_previous_ids = set()

    for overlap, current_id, previous_id in candidates:

        if current_id in id_mapping:
            continue

        if previous_id in used_previous_ids:
            continue

        id_mapping[current_id] = previous_id
        used_previous_ids.add(previous_id)

    # 対応する前時刻クラスタがなければ、新しいIDを割り当てる
    for current_id in sorted(current_groups):

        if current_id not in id_mapping:

            id_mapping[current_id] = next_id
            next_id += 1

    # 各ノードに対応付け後のIDを設定
    matched_clusters = {
        user_id: id_mapping[cluster_id]
        for user_id, cluster_id in zip(user_ids, cluster_labels)
    }

    return matched_clusters, next_id

# ============================================================
# 全時刻で推論・クラスタリング
# ============================================================

times = sorted(features_by_time.keys())

previous_clusters = {}
next_cluster_id = 0

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
        
        # 前時刻のクラスタIDを引き継ぐ
        matched_clusters, next_cluster_id = match_cluster_ids(
            user_ids,
            cluster_labels,
            previous_clusters,
            next_cluster_id
        )

        previous_clusters = matched_clusters.copy()

        # ログ出力
        for user_id in user_ids:

            fout.write(
                f"{current_time:g} "
                f"{user_id} "
                f"{matched_clusters[user_id]}\n"
            )

        print(
            f"Time={current_time:6.1f}s "
            f"Nodes={len(user_ids)} "
            f"Clusters={cluster_count}"
        )


print("\nClustering finished.")
print("Saved:", OUTPUT_FILE)