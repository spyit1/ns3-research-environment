import torch
from torch_geometric.data import Data
from torch_geometric.nn import SAGEConv


TRAINING_LOG_FILE = "graphsage_training.log"
EDGE_LOG_FILE = "graphsage_edge.log"

TARGET_TIME = 0.0


# ============================================================
# GraphSAGEモデル
# ============================================================

class GraphSAGE(torch.nn.Module):

    def __init__(self):
        super().__init__()

        # 入力2次元(x, y) → 中間16次元
        self.conv1 = SAGEConv(2, 16)

        # 中間16次元 → Embedding 8次元
        self.conv2 = SAGEConv(16, 8)

    def forward(self, x, edge_index):

        # 1層目
        x = self.conv1(x, edge_index)

        # 活性化関数
        x = torch.relu(x)

        # 2層目
        x = self.conv2(x, edge_index)

        return x

# ============================================================
# 1. ノード特徴量を読み込む
# ============================================================

nodes = []

with open(TRAINING_LOG_FILE, "r") as f:
    for line in f:
        parts = line.split()

        time = float(parts[0])
        user_id = int(parts[1])
        x = float(parts[2])
        y = float(parts[3])

        if time == TARGET_TIME:
            nodes.append((user_id, x, y))


# userId順に並べる
nodes.sort(key=lambda node: node[0])


# ============================================================
# 2. ノード特徴量 x を作成
# ============================================================

node_features = []

for user_id, x, y in nodes:
    node_features.append([x, y])


x = torch.tensor(
    node_features,
    dtype=torch.float
)


# ============================================================
# 3. Edgeを読み込む
# ============================================================

edges = []

with open(EDGE_LOG_FILE, "r") as f:
    for line in f:
        parts = line.split()

        time = float(parts[0])
        source = int(parts[1])
        target = int(parts[2])

        if time == TARGET_TIME:

            # 無向グラフなので両方向を登録
            edges.append([source, target])
            edges.append([target, source])


# ============================================================
# 4. edge_indexを作成
# ============================================================

edge_index = torch.tensor(
    edges,
    dtype=torch.long
).t().contiguous()


# ============================================================
# 5. PyTorch GeometricのDataを作成
# ============================================================

data = Data(
    x=x,
    edge_index=edge_index
)


# ============================================================
# 6. 確認
# ============================================================

print("=== PyTorch Geometric Data ===")

print(data)

print("\nNode features:")
print(data.x)

print("\nEdge index:")
print(data.edge_index)

print("\nx shape:")
print(data.x.shape)

print("\nedge_index shape:")
print(data.edge_index.shape)


# ============================================================
# 7. GraphSAGEに入力
# ============================================================

model = GraphSAGE()

model.eval()

with torch.no_grad():

    embeddings = model(
        data.x,
        data.edge_index
    )


# ============================================================
# 8. Embeddingを表示
# ============================================================

print("\n=== GraphSAGE Embeddings ===")

print(embeddings)

print("\nEmbedding shape:")
print(embeddings.shape)