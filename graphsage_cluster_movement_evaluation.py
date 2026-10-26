from collections import defaultdict
import csv


# ============================================================
# 設定
# ============================================================

GRAPH_SAGE_FILE = "graphsage_run6_cluster_fixed20.log"

BASELINE_FILE = "graphsage_run6_cluster_baseline_fixed20.log"

OUTPUT_FILE = "graphsage_cluster_movement_comparison_fixed20.csv"


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
# クラスタごとの所属ノードを取得
# ============================================================

def get_groups(clusters):

    groups = defaultdict(set)

    for user_id, cluster_id in clusters.items():

        groups[cluster_id].add(user_id)

    return groups


# ============================================================
# 前後のクラスタを対応付ける
# ============================================================

def match_clusters(previous_groups, current_groups):

    candidates = []

    for prev_id, prev_members in previous_groups.items():

        for curr_id, curr_members in current_groups.items():

            overlap = len(prev_members & curr_members)

            if overlap > 0:

                candidates.append(
                    (overlap, prev_id, curr_id)
                )

    # 共通ノード数が多い順に並べる
    candidates.sort(
        key=lambda x: (-x[0], x[1], x[2])
    )

    matched_previous = set()
    matched_current = set()

    matches = {}

    for overlap, prev_id, curr_id in candidates:

        if prev_id in matched_previous:
            continue

        if curr_id in matched_current:
            continue

        matches[curr_id] = prev_id

        matched_previous.add(prev_id)
        matched_current.add(curr_id)

    return matches


# ============================================================
# クラスタ移動回数の評価
# ============================================================

def evaluate(filename):

    clusters_by_time = load_clusters(filename)

    times = sorted(clusters_by_time)

    previous_clusters = None

    total_movements = 0

    results = []

    for current_time in times:

        current_clusters = clusters_by_time[current_time]

        current_groups = get_groups(current_clusters)

        movements = 0

        if previous_clusters is not None:

            previous_groups = get_groups(previous_clusters)

            matches = match_clusters(
                previous_groups,
                current_groups
            )

            common_users = (
                set(previous_clusters)
                & set(current_clusters)
            )

            for user_id in common_users:

                prev_id = previous_clusters[user_id]

                curr_id = current_clusters[user_id]

                matched_prev_id = matches.get(curr_id)

                if matched_prev_id != prev_id:

                    movements += 1

        total_movements += movements

        results.append({
            "time": current_time,
            "movements": movements,
            "cumulative_movements": total_movements,
            "clusters": len(current_groups)
        })

        previous_clusters = current_clusters

    return results, total_movements


# ============================================================
# GraphSAGEあり・なしを評価
# ============================================================

sage_results, sage_total = evaluate(
    GRAPH_SAGE_FILE
)

baseline_results, baseline_total = evaluate(
    BASELINE_FILE
)


# ============================================================
# CSV出力
# ============================================================

if [r["time"] for r in sage_results] != [
    r["time"] for r in baseline_results
]:
    raise ValueError("Evaluation times do not match.")


with open(OUTPUT_FILE, "w", newline="") as f:

    writer = csv.writer(f)

    writer.writerow([
        "time",
        "graphsage_movements",
        "baseline_movements",
        "graphsage_cumulative",
        "baseline_cumulative",
        "graphsage_clusters",
        "baseline_clusters"
    ])

    for sage, baseline in zip(
        sage_results,
        baseline_results
    ):

        writer.writerow([
            sage["time"],
            sage["movements"],
            baseline["movements"],
            sage["cumulative_movements"],
            baseline["cumulative_movements"],
            sage["clusters"],
            baseline["clusters"]
        ])


# ============================================================
# 結果表示
# ============================================================

print("\n=== Cluster Movement Comparison ===")

print("\nGraphSAGE:")
print("Total movements:", sage_total)

print("\nBaseline:")
print("Total movements:", baseline_total)

if baseline_total > 0:

    reduction = (
        (baseline_total - sage_total)
        / baseline_total
        * 100
    )

    print(
        "\nMovement reduction rate:",
        f"{reduction:.2f}%"
    )

print("\nSaved:", OUTPUT_FILE)