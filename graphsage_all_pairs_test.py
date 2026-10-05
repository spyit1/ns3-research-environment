import math


TRAINING_LOG_FILE = "graphsage_training.log"

FEATURE_INTERVAL = 5.0
FUTURE_DELTA = 30.0
SIMULATION_TIME = 200.0

DISTANCE_THRESHOLD = 100.0


# ============================================================
# 全時刻の位置情報を読み込む
# ============================================================

positions_by_time = {}

with open(TRAINING_LOG_FILE, "r") as f:

    for line in f:

        parts = line.split()

        time = float(parts[0])
        user_id = int(parts[1])
        x = float(parts[2])
        y = float(parts[3])

        if time not in positions_by_time:
            positions_by_time[time] = {}

        positions_by_time[time][user_id] = (x, y)


# ============================================================
# 距離計算
# ============================================================

def calculate_distance(pos1, pos2):

    x1, y1 = pos1
    x2, y2 = pos2

    return math.sqrt(
        (x1 - x2) ** 2 +
        (y1 - y2) ** 2
    )


# ============================================================
# 全時刻についてPositive / Negative Pairを作成
# ============================================================

all_positive_pairs = []
all_negative_pairs = []

current_time = 0.0


while current_time + FUTURE_DELTA <= SIMULATION_TIME:

    future_time = current_time + FUTURE_DELTA

    # 必要な時刻のデータが存在しない場合はスキップ
    if (
        current_time not in positions_by_time
        or future_time not in positions_by_time
    ):
        print(
            f"time={current_time:.0f} "
            f"future={future_time:.0f} "
            f"skipped (data not found)"
        )

        current_time += FEATURE_INTERVAL
        continue

    current_positions = positions_by_time[current_time]
    future_positions = positions_by_time[future_time]

    user_ids = sorted(current_positions.keys())

    positive_pairs = []
    negative_pairs = []

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

            if (
                current_distance <= DISTANCE_THRESHOLD
                and
                future_distance <= DISTANCE_THRESHOLD
            ):

                positive_pairs.append(
                    (user1, user2)
                )

                all_positive_pairs.append(
                    (current_time, user1, user2)
                )

            else:

                negative_pairs.append(
                    (user1, user2)
                )

                all_negative_pairs.append(
                    (current_time, user1, user2)
                )

    print(
        f"time={current_time:.0f} "
        f"future={future_time:.0f} "
        f"positive={len(positive_pairs)} "
        f"negative={len(negative_pairs)}"
    )

    current_time += FEATURE_INTERVAL


# ============================================================
# 全体の結果
# ============================================================

print("\n=== Summary ===")

print(
    "Positive pairs:",
    len(all_positive_pairs)
)

print(
    "Negative pairs:",
    len(all_negative_pairs)
)