TRAINING_LOG_FILE = "graphsage_training.log"

CURRENT_TIME = 0.0
FUTURE_DELTA = 30.0

FUTURE_TIME = CURRENT_TIME + FUTURE_DELTA


# ============================================================
# 指定した時刻のノード位置を取得
# ============================================================

def load_positions(target_time):

    positions = {}

    with open(TRAINING_LOG_FILE, "r") as f:

        for line in f:

            parts = line.split()

            time = float(parts[0])
            user_id = int(parts[1])
            x = float(parts[2])
            y = float(parts[3])

            if time == target_time:
                positions[user_id] = (x, y)

    return positions


# ============================================================
# 現在と30秒後の位置を取得
# ============================================================

current_positions = load_positions(CURRENT_TIME)
future_positions = load_positions(FUTURE_TIME)


# ============================================================
# 確認
# ============================================================

print("=== Current:", CURRENT_TIME, "sec ===")

for user_id in sorted(current_positions):
    x, y = current_positions[user_id]

    print(
        f"userId={user_id} "
        f"x={x} y={y}"
    )


print("\n=== Future:", FUTURE_TIME, "sec ===")

for user_id in sorted(future_positions):
    x, y = future_positions[user_id]

    print(
        f"userId={user_id} "
        f"x={x} y={y}"
    )
    
    
import math


# ============================================================
# 2ノード間の距離を計算
# ============================================================

def calculate_distance(pos1, pos2):

    x1, y1 = pos1
    x2, y2 = pos2

    return math.sqrt(
        (x1 - x2) ** 2 +
        (y1 - y2) ** 2
    )


# ============================================================
# Positive / Negative Pairを作成
# ============================================================

DISTANCE_THRESHOLD = 100.0

positive_pairs = []
negative_pairs = []

user_ids = sorted(current_positions.keys())


for i in range(len(user_ids)):

    for j in range(i + 1, len(user_ids)):

        user1 = user_ids[i]
        user2 = user_ids[j]

        # 現在の距離
        current_distance = calculate_distance(
            current_positions[user1],
            current_positions[user2]
        )

        # 30秒後の距離
        future_distance = calculate_distance(
            future_positions[user1],
            future_positions[user2]
        )

        # 現在も近く、30秒後も近い
        if (
            current_distance <= DISTANCE_THRESHOLD
            and
            future_distance <= DISTANCE_THRESHOLD
        ):

            positive_pairs.append(
                (user1, user2)
            )

        else:

            negative_pairs.append(
                (user1, user2)
            )


# ============================================================
# 結果表示
# ============================================================

print("\n=== Positive Pairs ===")

for user1, user2 in positive_pairs:
    print(f"{user1} -- {user2}")


print("\n=== Negative Pairs ===")

for user1, user2 in negative_pairs:
    print(f"{user1} -- {user2}")