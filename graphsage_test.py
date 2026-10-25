import math
import torch
import torch.nn.functional as F

from torch_geometric.nn import SAGEConv
from sklearn.cluster import KMeans


# ============================================================
# 設定
# ============================================================

MODEL_FILE = "graphsage_50nodes_run5_500epochs.pth"

TRAINING_LOG_FILE = "graphsage_training.log"
EDGE_LOG_FILE = "graphsage_edge.log"

POSITION_SCALE = 1000.0
VELOCITY_SCALE = 5.0

FUTURE_DELTA = 30.0
DISTANCE_THRESHOLD = 100.0

TEST_TIME = 0.0
NUM_CLUSTERS = 3


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
# 位置間距離
# ============================================================

def calculate_distance(pos1, pos2):

    return math.sqrt(
        (pos1[0] - pos2[0]) ** 2
        +
        (pos1[1] - pos2[1]) ** 2
    )


# ============================================================
# ログ読み込み
# ============================================================

positions_by_time = {}
velocities_by_time = {}
destinations_by_time = {}

with open(TRAINING_LOG_FILE, "r") as f:

    for line in f:

        parts = line.split()

        time = float(parts[0])
        user_id = int(parts[1])

        x = float(parts[2])
        y = float(parts[3])

        vx = float(parts[4])
        vy = float(parts[5])

        dest_x = float(parts[6])
        dest_y = float(parts[7])

        positions_by_time.setdefault(time, {})[user_id] = (x, y)

        velocities_by_time.setdefault(time, {})[user_id] = (vx, vy)

        destinations_by_time.setdefault(time, {})[user_id] = (
            dest_x,
            dest_y
        )


# ============================================================
# Edgeログ読み込み
# ============================================================

edges_by_time = {}

with open(EDGE_LOG_FILE, "r") as f:

    for line in f:

        parts = line.split()

        time = float(parts[0])
        source = int(parts[1])
        target = int(parts[2])

        edges_by_time.setdefault(time, []).append(
            (source, target)
        )

        edges_by_time[time].append(
            (target, source)
        )


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
# 時刻0秒のノード情報
# ============================================================

current_positions = positions_by_time[TEST_TIME]

user_ids = sorted(current_positions.keys())

user_to_index = {
    user_id: index
    for index, user_id in enumerate(user_ids)
}


# ============================================================
# Node feature
# ============================================================

node_features = []

for user_id in user_ids:

    x, y = current_positions[user_id]

    vx, vy = velocities_by_time[TEST_TIME][user_id]

    dest_x, dest_y = destinations_by_time[TEST_TIME][user_id]

    node_features.append(
        [
            x / POSITION_SCALE,
            y / POSITION_SCALE,
            vx / VELOCITY_SCALE,
            vy / VELOCITY_SCALE,
            dest_x / POSITION_SCALE,
            dest_y / POSITION_SCALE
        ]
    )

x_tensor = torch.tensor(
    node_features,
    dtype=torch.float
)


# ============================================================
# edge_index
# ============================================================

edge_list = []

for source, target in edges_by_time.get(TEST_TIME, []):

    if source in user_to_index and target in user_to_index:

        edge_list.append(
            [
                user_to_index[source],
                user_to_index[target]
            ]
        )

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


# ============================================================
# GraphSAGE推論（学習なし）
# ============================================================

with torch.no_grad():

    embeddings = model(
        x_tensor,
        edge_index
    )


# ============================================================
# Embedding表示
# ============================================================

print("\n=== Pretrained Embeddings ===")

for i, user_id in enumerate(user_ids):

    print(
        f"userId={user_id}: "
        f"{embeddings[i].tolist()}"
    )



# ============================================================
# 30秒後の近接関係で評価
# ============================================================

future_time = TEST_TIME + FUTURE_DELTA

future_positions = positions_by_time[future_time]

print("\n=== Future Pair Evaluation ===")

correct_pairs = 0
total_pairs = 0

TP = 0
FP = 0
TN = 0
FN = 0

for i in range(len(user_ids)):

    for j in range(i + 1, len(user_ids)):

        user1 = user_ids[i]
        user2 = user_ids[j]

        current_distance = calculate_distance(
            current_positions[user1],
            current_positions[user2]
        )

        future_distance = calculate_distance(
            future_positions[user1],
            future_positions[user2]
        )

        # 正解ラベル
        actual_positive = (
            current_distance <= DISTANCE_THRESHOLD
            and
            future_distance <= DISTANCE_THRESHOLD
        )

        # Embedding間距離
        embedding_distance = torch.norm(
            embeddings[i] - embeddings[j]
        ).item()

        # 予測ラベル
        predicted_positive = (
            embedding_distance < 1.0
        )

        if actual_positive and predicted_positive:
            TP += 1

        elif not actual_positive and predicted_positive:
            FP += 1

        elif not actual_positive and not predicted_positive:
            TN += 1

        else:
            FN += 1

        if actual_positive == predicted_positive:
            correct_pairs += 1

        total_pairs += 1

        print(
            f"{user1} -- {user2} "
            f"Actual={'Positive' if actual_positive else 'Negative'} "
            f"Predicted={'Positive' if predicted_positive else 'Negative'} "
            f"FutureDistance={future_distance:.3f}"
        )




# ============================================================
# 評価指標の計算
# ============================================================

accuracy = (
    (TP + TN) / (TP + FP + TN + FN)
    if (TP + FP + TN + FN) > 0
    else 0.0
)

precision = (
    TP / (TP + FP)
    if (TP + FP) > 0
    else 0.0
)

recall = (
    TP / (TP + FN)
    if (TP + FN) > 0
    else 0.0
)

f1_score = (
    2 * precision * recall / (precision + recall)
    if (precision + recall) > 0
    else 0.0
)


print("\n=== Evaluation Result ===")

print("TP:", TP)
print("FP:", FP)
print("TN:", TN)
print("FN:", FN)

print("Total pairs:", total_pairs)

print(f"Accuracy : {accuracy * 100:.2f}%")
print(f"Precision: {precision * 100:.2f}%")
print(f"Recall   : {recall * 100:.2f}%")
print(f"F1-score : {f1_score * 100:.2f}%")