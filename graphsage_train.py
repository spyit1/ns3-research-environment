import math

import torch
import torch.nn.functional as F

from torch_geometric.data import Data
from torch_geometric.nn import SAGEConv

from sklearn.cluster import KMeans


# ============================================================
# 設定
# ============================================================

TRAINING_LOG_FILE = "graphsage_training.log"
EDGE_LOG_FILE = "graphsage_edge.log"

FEATURE_INTERVAL = 5.0
FUTURE_DELTA = 30.0

DISTANCE_THRESHOLD = 100.0

# 今回のフィールドサイズ
POSITION_SCALE = 1000.0

# 移動速度の正規化用
VELOCITY_SCALE = 5.0

EPOCHS = 100
LEARNING_RATE = 0.01

# 再現性確認用
torch.manual_seed(0)


# ============================================================
# GraphSAGEモデル
# ============================================================

class GraphSAGE(torch.nn.Module):

    def __init__(self):

        super().__init__()

        # x,y,vx,vy の4次元
        # ↓
        # 16次元
        self.conv1 = SAGEConv(4, 16)

        # 16次元
        # ↓
        # 8次元Embedding
        self.conv2 = SAGEConv(16, 8)

    def forward(self, x, edge_index):

        x = self.conv1(x, edge_index)

        x = F.relu(x)

        x = self.conv2(x, edge_index)

        return x


# ============================================================
# 距離計算
# ============================================================

def calculate_distance(pos1, pos2):

    x1, y1 = pos1
    x2, y2 = pos2

    return math.sqrt(
        (x1 - x2) ** 2
        +
        (y1 - y2) ** 2
    )


# ============================================================
# 位置・速度ログ読み込み
# ============================================================

positions_by_time = {}
velocities_by_time = {}

with open(TRAINING_LOG_FILE, "r") as f:

    for line in f:

        parts = line.split()

        time = float(parts[0])
        user_id = int(parts[1])

        x = float(parts[2])
        y = float(parts[3])

        vx = float(parts[4])
        vy = float(parts[5])

        if time not in positions_by_time:
            positions_by_time[time] = {}

        if time not in velocities_by_time:
            velocities_by_time[time] = {}

        positions_by_time[time][user_id] = (
            x,
            y
        )

        velocities_by_time[time][user_id] = (
            vx,
            vy
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

        if time not in edges_by_time:
            edges_by_time[time] = []

        # 無向グラフなので両方向
        edges_by_time[time].append(
            (source, target)
        )

        edges_by_time[time].append(
            (target, source)
        )


# ============================================================
# 学習に使用できる時刻を取得
# ============================================================

training_times = []

for current_time in sorted(positions_by_time.keys()):

    future_time = current_time + FUTURE_DELTA

    if (
        future_time in positions_by_time
        and
        current_time in edges_by_time
    ):
        training_times.append(current_time)


print(
    "Training time count:",
    len(training_times)
)


# ============================================================
# GraphSAGE
# ============================================================

model = GraphSAGE()

optimizer = torch.optim.Adam(
    model.parameters(),
    lr=LEARNING_RATE
)


# ============================================================
# 学習
# ============================================================

for epoch in range(EPOCHS):

    model.train()

    optimizer.zero_grad()

    total_loss = 0.0


    for current_time in training_times:

        future_time = (
            current_time
            +
            FUTURE_DELTA
        )

        current_positions = (
            positions_by_time[current_time]
        )

        future_positions = (
            positions_by_time[future_time]
        )

        user_ids = sorted(
            current_positions.keys()
        )


        # ====================================================
        # userId → Tensor内部index
        # ====================================================

        user_to_index = {
            user_id: index
            for index, user_id
            in enumerate(user_ids)
        }


        # ====================================================
        # Node feature
        # ====================================================

        node_features = []

        for user_id in user_ids:

            x, y = current_positions[user_id]

            vx, vy = velocities_by_time[current_time][user_id]

            # 位置と速度をそれぞれ正規化
            node_features.append(
                [
                    x / POSITION_SCALE,
                    y / POSITION_SCALE,
                    vx / VELOCITY_SCALE,
                    vy / VELOCITY_SCALE
                ]
            )


        x_tensor = torch.tensor(
            node_features,
            dtype=torch.float
        )


        # ====================================================
        # edge_index
        # ====================================================

        edge_list = []

        for source, target in edges_by_time[current_time]:

            if (
                source in user_to_index
                and
                target in user_to_index
            ):

                edge_list.append(
                    [
                        user_to_index[source],
                        user_to_index[target]
                    ]
                )


        edge_index = torch.tensor(
            edge_list,
            dtype=torch.long
        ).t().contiguous()


        # ====================================================
        # PyG Data
        # ====================================================

        data = Data(
            x=x_tensor,
            edge_index=edge_index
        )


        # ====================================================
        # GraphSAGE
        # ====================================================

        embeddings = model(
            data.x,
            data.edge_index
        )


        # ====================================================
        # Positive / Negative Pair Loss
        # ====================================================

        pair_loss = 0.0
        pair_count = 0


        for i in range(len(user_ids)):

            for j in range(
                i + 1,
                len(user_ids)
            ):

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


                # Embedding間距離
                embedding_distance = torch.norm(
                    embeddings[i]
                    -
                    embeddings[j]
                )


                # ============================================
                # Positive
                # ============================================

                if (
                    current_distance
                    <= DISTANCE_THRESHOLD
                    and
                    future_distance
                    <= DISTANCE_THRESHOLD
                ):

                    # PositiveはEmbeddingを近づける
                    loss = (
                        embedding_distance ** 2
                    )


                # ============================================
                # Negative
                # ============================================

                else:

                    # Negativeは最低1.0離す
                    margin = 1.0

                    loss = (
                        torch.clamp(
                            margin
                            -
                            embedding_distance,
                            min=0.0
                        )
                        ** 2
                    )


                pair_loss = (
                    pair_loss
                    +
                    loss
                )

                pair_count += 1


        if pair_count > 0:

            pair_loss = (
                pair_loss
                /
                pair_count
            )

            total_loss = (
                total_loss
                +
                pair_loss
            )


    # ========================================================
    # 全時刻の平均Loss
    # ========================================================

    total_loss = (
        total_loss
        /
        len(training_times)
    )


    # ========================================================
    # Backpropagation
    # ========================================================

    total_loss.backward()

    optimizer.step()


    # ========================================================
    # Loss表示
    # ========================================================

    if (
        epoch == 0
        or
        (epoch + 1) % 10 == 0
    ):

        print(
            f"Epoch {epoch + 1:3d} "
            f"Loss = "
            f"{total_loss.item():.6f}"
        )


print("\nTraining finished.")

# ============================================================
# 学習後Embeddingの確認
# ============================================================

model.eval()

TEST_TIME = 0.0

test_positions = positions_by_time[TEST_TIME]

user_ids = sorted(
    test_positions.keys()
)


# ============================================================
# userId → Tensor内部index
# ============================================================

user_to_index = {
    user_id: index
    for index, user_id
    in enumerate(user_ids)
}


# ============================================================
# Node feature
# ============================================================

node_features = []

for user_id in user_ids:

    x, y = test_positions[user_id]

    vx, vy = velocities_by_time[TEST_TIME][user_id]

    node_features.append(
        [
            x / POSITION_SCALE,
            y / POSITION_SCALE,
            vx / VELOCITY_SCALE,
            vy / VELOCITY_SCALE
        ]
    )


x_tensor = torch.tensor(
    node_features,
    dtype=torch.float
)


# ============================================================
# Edge
# ============================================================

edge_list = []

for source, target in edges_by_time[TEST_TIME]:

    if (
        source in user_to_index
        and
        target in user_to_index
    ):

        edge_list.append(
            [
                user_to_index[source],
                user_to_index[target]
            ]
        )


edge_index = torch.tensor(
    edge_list,
    dtype=torch.long
).t().contiguous()


# ============================================================
# 学習済みGraphSAGEでEmbedding生成
# ============================================================

with torch.no_grad():

    embeddings = model(
        x_tensor,
        edge_index
    )


# ============================================================
# Embedding表示
# ============================================================

print("\n=== Trained Embeddings ===")

for i, user_id in enumerate(user_ids):

    print(
        f"userId={user_id}: "
        f"{embeddings[i].tolist()}"
    )
    
    
# ============================================================
# Embedding間距離を確認
# ============================================================

print("\n=== Embedding Distances ===")

for i in range(len(user_ids)):

    for j in range(i + 1, len(user_ids)):

        distance = torch.norm(
            embeddings[i] - embeddings[j]
        ).item()

        print(
            f"{user_ids[i]} -- {user_ids[j]} "
            f": {distance:.6f}"
        )
        
# ============================================================
# K-meansによるクラスタリング
# ============================================================

NUM_CLUSTERS = 3

# PyTorch Tensor → NumPy
embedding_array = (
    embeddings
    .detach()
    .cpu()
    .numpy()
)

kmeans = KMeans(
    n_clusters=NUM_CLUSTERS,
    random_state=0,
    n_init=10
)

cluster_labels = kmeans.fit_predict(
    embedding_array
)


# ============================================================
# クラスタリング結果
# ============================================================

print("\n=== Clustering Result ===")

for i, user_id in enumerate(user_ids):

    print(
        f"userId={user_id} "
        f"cluster={cluster_labels[i]}"
    )
    
    
# ============================================================
# 時刻別Pair評価
# ============================================================

print("\n=== Pair Evaluation ===")

EVALUATION_TIMES = [60.0, 90.0]

for eval_time in EVALUATION_TIMES:

    future_time = eval_time + FUTURE_DELTA

    current_positions = positions_by_time[eval_time]
    future_positions = positions_by_time[future_time]

    eval_user_ids = sorted(
        current_positions.keys()
    )

    eval_user_to_index = {
        user_id: index
        for index, user_id in enumerate(eval_user_ids)
    }

    # Node feature
    eval_node_features = []

    for user_id in eval_user_ids:

        x, y = current_positions[user_id]

        vx, vy = velocities_by_time[eval_time][user_id]

        eval_node_features.append(
            [
                x / POSITION_SCALE,
                y / POSITION_SCALE,
                vx / VELOCITY_SCALE,
                vy / VELOCITY_SCALE
            ]
        )

    eval_x_tensor = torch.tensor(
        eval_node_features,
        dtype=torch.float
    )

    # Edge
    eval_edge_list = []

    for source, target in edges_by_time[eval_time]:

        if (
            source in eval_user_to_index
            and
            target in eval_user_to_index
        ):

            eval_edge_list.append(
                [
                    eval_user_to_index[source],
                    eval_user_to_index[target]
                ]
            )

    eval_edge_index = torch.tensor(
        eval_edge_list,
        dtype=torch.long
    ).t().contiguous()

    # Embedding
    with torch.no_grad():

        eval_embeddings = model(
            eval_x_tensor,
            eval_edge_index
        )

    print(
        f"\nTime {eval_time:.0f} "
        f"-> {future_time:.0f}"
    )

    # 今回確認したいGroup Aだけ評価
    target_pairs = [
        (0, 1),
        (0, 2),
        (1, 2)
    ]

    for user1, user2 in target_pairs:

        current_distance = calculate_distance(
            current_positions[user1],
            current_positions[user2]
        )

        future_distance = calculate_distance(
            future_positions[user1],
            future_positions[user2]
        )

        i = eval_user_to_index[user1]
        j = eval_user_to_index[user2]

        embedding_distance = torch.norm(
            eval_embeddings[i]
            -
            eval_embeddings[j]
        ).item()

        if (
            current_distance <= DISTANCE_THRESHOLD
            and
            future_distance <= DISTANCE_THRESHOLD
        ):
            pair_type = "Positive"
        else:
            pair_type = "Negative"

        print(
            f"{user1} -- {user2} "
            f"{pair_type} "
            f"current={current_distance:.3f} "
            f"future={future_distance:.3f} "
            f"embedding={embedding_distance:.6f}"
        )
        

# ============================================================
# 全時刻のクラスタリング結果をファイル出力
# ============================================================

CLUSTER_LOG_FILE = "graphsage_cluster.log"

print("\n=== Export Cluster Log ===")

with open(CLUSTER_LOG_FILE, "w") as cluster_file:

    for cluster_time in sorted(positions_by_time.keys()):

        cluster_positions = positions_by_time[cluster_time]

        cluster_user_ids = sorted(
            cluster_positions.keys()
        )

        cluster_user_to_index = {
            user_id: index
            for index, user_id in enumerate(cluster_user_ids)
        }

        # ====================================================
        # Node feature
        # ====================================================

        cluster_node_features = []

        for user_id in cluster_user_ids:

            x, y = cluster_positions[user_id]

            vx, vy = velocities_by_time[cluster_time][user_id]

            cluster_node_features.append(
                [
                    x / POSITION_SCALE,
                    y / POSITION_SCALE,
                    vx / VELOCITY_SCALE,
                    vy / VELOCITY_SCALE
                ]
            )
        
        cluster_x_tensor = torch.tensor(
            cluster_node_features,
            dtype=torch.float
        )

        # ====================================================
        # Edge
        # ====================================================

        cluster_edge_list = []

        for source, target in edges_by_time[cluster_time]:

            if (
                source in cluster_user_to_index
                and
                target in cluster_user_to_index
            ):

                cluster_edge_list.append(
                    [
                        cluster_user_to_index[source],
                        cluster_user_to_index[target]
                    ]
                )

        cluster_edge_index = torch.tensor(
            cluster_edge_list,
            dtype=torch.long
        ).t().contiguous()

        # ====================================================
        # 学習済みGraphSAGE
        # ====================================================

        with torch.no_grad():

            cluster_embeddings = model(
                cluster_x_tensor,
                cluster_edge_index
            )

        # ====================================================
        # K-means
        # ====================================================

        cluster_embedding_array = (
            cluster_embeddings
            .detach()
            .cpu()
            .numpy()
        )

        cluster_kmeans = KMeans(
            n_clusters=NUM_CLUSTERS,
            random_state=0,
            n_init=10
        )

        cluster_labels = cluster_kmeans.fit_predict(
            cluster_embedding_array
        )

        # ====================================================
        # ファイル出力
        # time userId clusterId
        # ====================================================

        for i, user_id in enumerate(cluster_user_ids):

            cluster_file.write(
                f"{cluster_time:.0f} "
                f"{user_id} "
                f"{cluster_labels[i]}\n"
            )


print(
    f"Saved: {CLUSTER_LOG_FILE}"
)