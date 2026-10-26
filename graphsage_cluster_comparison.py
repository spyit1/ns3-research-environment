from collections import defaultdict
import csv


# ============================================================
# 設定
# ============================================================

GRAPH_SAGE_FILE = "graphsage_run6_cluster_fixed20.log"

BASELINE_FILE = "graphsage_run6_cluster_baseline_fixed20.log"

OUTPUT_FILE = "graphsage_cluster_comparison_fixed20.csv"


# ============================================================
# クラスタログ読み込み
# ============================================================

def load_clusters(filename):

    clusters_by_time = defaultdict(dict)

    with open(filename, "r") as f:

        for line in f:

            parts = line.split()

            if len(parts) != 3:
                continue

            current_time = float(parts[0])
            user_id = int(parts[1])
            cluster_id = int(parts[2])

            clusters_by_time[current_time][user_id] = cluster_id

    return clusters_by_time


# ============================================================
# 各ノードの所属メンバーを取得
# ============================================================

def get_memberships(clusters):

    groups = defaultdict(set)

    for user_id, cluster_id in clusters.items():
        groups[cluster_id].add(user_id)

    memberships = {}

    for members in groups.values():

        for user_id in members:
            memberships[user_id] = frozenset(members)

    return memberships


# ============================================================
# 評価
# ============================================================

def evaluate(filename):

    clusters_by_time = load_clusters(filename)

    times = sorted(clusters_by_time.keys())

    previous_memberships = None

    total_changes = 0
    total_clusters = 0

    results = []

    for current_time in times:

        clusters = clusters_by_time[current_time]

        memberships = get_memberships(clusters)

        cluster_count = len(set(clusters.values()))

        total_clusters += cluster_count

        changes = 0

        if previous_memberships is not None:

            for user_id in memberships:

                if user_id not in previous_memberships:
                    continue

                if (
                    memberships[user_id]
                    != previous_memberships[user_id]
                ):
                    changes += 1

        total_changes += changes

        results.append({
            "time": current_time,
            "clusters": cluster_count,
            "changes": changes,
            "cumulative_changes": total_changes
        })

        previous_memberships = memberships

    average_clusters = total_clusters / len(times)

    return results, total_changes, average_clusters


# ============================================================
# 両手法を評価
# ============================================================

sage_results, sage_changes, sage_avg_clusters = evaluate(
    GRAPH_SAGE_FILE
)

baseline_results, baseline_changes, baseline_avg_clusters = evaluate(
    BASELINE_FILE
)


# ============================================================
# CSV出力
# ============================================================

with open(OUTPUT_FILE, "w", newline="") as f:

    writer = csv.writer(f)

    writer.writerow([
        "time",
        "graphsage_clusters",
        "baseline_clusters",
        "graphsage_changes",
        "baseline_changes",
        "graphsage_cumulative_changes",
        "baseline_cumulative_changes"
    ])

    for sage, baseline in zip(sage_results, baseline_results):

        if sage["time"] != baseline["time"]:
            raise ValueError("Evaluation times do not match.")

        writer.writerow([
            sage["time"],
            sage["clusters"],
            baseline["clusters"],
            sage["changes"],
            baseline["changes"],
            sage["cumulative_changes"],
            baseline["cumulative_changes"]
        ])


# ============================================================
# 結果表示
# ============================================================

print("\n=== Cluster Comparison ===")

print("\nGraphSAGE:")
print("Total membership changes:", sage_changes)
print("Average cluster count:", round(sage_avg_clusters, 2))

print("\nBaseline:")
print("Total membership changes:", baseline_changes)
print("Average cluster count:", round(baseline_avg_clusters, 2))

if baseline_changes > 0:

    reduction = (
        (baseline_changes - sage_changes)
        / baseline_changes
        * 100
    )

    print(
        "\nChange reduction rate:",
        f"{reduction:.2f}%"
    )

print("\nSaved:", OUTPUT_FILE)