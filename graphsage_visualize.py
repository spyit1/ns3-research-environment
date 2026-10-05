import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import defaultdict


# ============================================================
# 設定
# ============================================================

FEATURE_FILE = "graphsage_training.log"
CLUSTER_FILE = "graphsage_cluster.log"

EDGE_FILE = "graphsage_edge.log"

OUTPUT_FILE = "graphsage_cluster_movement.gif"

X_MIN = 0
X_MAX = 800

Y_MIN = 0
Y_MAX = 950


# ============================================================
# 位置情報読み込み
# time userId x y
# ============================================================

positions_by_time = defaultdict(dict)

with open(FEATURE_FILE, "r") as f:

    for line in f:

        line = line.strip()

        if not line:
            continue

        parts = line.split()

        if len(parts) != 4:
            continue

        time = float(parts[0])
        user_id = int(parts[1])
        x = float(parts[2])
        y = float(parts[3])

        positions_by_time[time][user_id] = (x, y)


# ============================================================
# クラスタ情報読み込み
# time userId clusterId
# ============================================================

clusters_by_time = defaultdict(dict)

with open(CLUSTER_FILE, "r") as f:

    for line in f:

        line = line.strip()

        if not line:
            continue

        parts = line.split()

        if len(parts) != 3:
            continue

        time = float(parts[0])
        user_id = int(parts[1])
        cluster_id = int(parts[2])

        clusters_by_time[time][user_id] = cluster_id
        

# ============================================================
# Edge情報読み込み
# time source target
# ============================================================

edges_by_time = defaultdict(list)

with open(EDGE_FILE, "r") as f:

    for line in f:

        line = line.strip()

        if not line:
            continue

        parts = line.split()

        if len(parts) != 3:
            continue

        time = float(parts[0])
        source = int(parts[1])
        target = int(parts[2])

        edges_by_time[time].append(
            (source, target)
        )


# ============================================================
# 時刻一覧
# ============================================================

times = sorted(positions_by_time.keys())

print("Time count:", len(times))
print("Start time:", times[0])
print("End time:", times[-1])


# ============================================================
# 色設定
# ============================================================

CLUSTER_COLORS = {
    0: "tab:blue",
    1: "tab:orange",
    2: "tab:green"
}


# ============================================================
# グラフ
# ============================================================

fig, ax = plt.subplots(
    figsize=(8, 8)
)

ax.set_xlim(
    X_MIN,
    X_MAX
)

ax.set_ylim(
    Y_MIN,
    Y_MAX
)

ax.set_xlabel("X [m]")
ax.set_ylabel("Y [m]")

ax.grid(True)


# ============================================================
# ノード
# ============================================================

scatter = ax.scatter(
    [],
    [],
    s=180,
    zorder=2
)

labels = []
edge_lines = []

# ============================================================
# 更新
# ============================================================

def update(frame):

    global labels
    global edge_lines

    current_time = times[frame]

    positions = positions_by_time[
        current_time
    ]

    clusters = clusters_by_time.get(
        current_time,
        {}
    )

    user_ids = sorted(
        positions.keys()
    )

    xs = []
    ys = []
    colors = []

    for user_id in user_ids:

        x, y = positions[user_id]

        xs.append(x)
        ys.append(y)

        cluster_id = clusters.get(
            user_id,
            -1
        )

        color = CLUSTER_COLORS.get(
            cluster_id,
            "gray"
        )

        colors.append(color)
        
    # ========================================================
    # 前フレームのEdgeを削除
    # ========================================================

    for line in edge_lines:

        line.remove()

    edge_lines = []


    # ========================================================
    # 現在時刻のEdgeを描画
    # ========================================================

    current_edges = edges_by_time.get(
        current_time,
        []
    )

    for source, target in current_edges:

        if (
            source not in positions
            or
            target not in positions
        ):
            continue

        x1, y1 = positions[source]
        x2, y2 = positions[target]

        line, = ax.plot(
            [x1, x2],
            [y1, y2],
            color="gray",
            linewidth=1.5,
            alpha=0.6,
            zorder=1
        )

        edge_lines.append(line)

    # ========================================================
    # 位置更新
    # ========================================================

    scatter.set_offsets(
        list(zip(xs, ys))
    )

    scatter.set_color(
        colors
    )


    # ========================================================
    # 古いラベル削除
    # ========================================================

    for label in labels:

        label.remove()

    labels = []


    # ========================================================
    # userId + clusterId
    # ========================================================

    for user_id in user_ids:

        x, y = positions[user_id]

        cluster_id = clusters.get(
            user_id,
            -1
        )

        label = ax.text(
            x + 8,
            y + 8,
            f"{user_id} (C{cluster_id})",
            fontsize=10
        )

        labels.append(label)


    # ========================================================
    # タイトル
    # ========================================================

    ax.set_title(
        "GraphSAGE Clustering\n"
        f"Time = {current_time:.0f} s"
    )

    return edge_lines + [scatter] + labels


# ============================================================
# アニメーション
# ============================================================

animation = FuncAnimation(
    fig,
    update,
    frames=len(times),
    interval=500,
    repeat=True
)


# ============================================================
# GIF保存
# ============================================================

print("Saving animation...")

animation.save(
    OUTPUT_FILE,
    writer="pillow",
    fps=2
)

print(
    f"Saved: {OUTPUT_FILE}"
)