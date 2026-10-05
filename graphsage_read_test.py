TRAINING_LOG_FILE = "graphsage_training.log"
EDGE_LOG_FILE = "graphsage_edge.log"

TARGET_TIME = 0.0


# -----------------------------
# ノード特徴量を読み込む
# -----------------------------
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


# -----------------------------
# Edgeを読み込む
# -----------------------------
edges = []

with open(EDGE_LOG_FILE, "r") as f:
    for line in f:
        parts = line.split()

        time = float(parts[0])
        source = int(parts[1])
        target = int(parts[2])

        if time == TARGET_TIME:
            edges.append((source, target))


# -----------------------------
# 結果表示
# -----------------------------
print("=== Time", TARGET_TIME, "===")

print("\nNodes:")

for user_id, x, y in nodes:
    print(
        f"userId={user_id} "
        f"x={x} "
        f"y={y}"
    )

print("\nEdges:")

for source, target in edges:
    print(f"{source} -- {target}")