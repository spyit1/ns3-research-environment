import csv

def load_csv(filename):
    """CSV を userID → time の辞書として読み込む"""
    data = {}
    with open(filename, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)

        # 列名の空白を除去
        field_map = {name.strip(): name for name in reader.fieldnames}

        uid_key = field_map.get("userID")
        time_key = field_map.get("time")

        if uid_key is None or time_key is None:
            raise ValueError(f"列名エラー: {reader.fieldnames}")

        for row in reader:
            uid = int(row[uid_key])
            t = float(row[time_key])
            data[uid] = t

    return data


def fmt(x):
    """指数表記を使わず丸める（小数2～4桁）"""
    if x == 0:
        return "0"
    if abs(x) >= 1:
        return f"{x:.2f}"
    else:
        return f"{x:.4f}"


def compare_times(old_file, new_file):
    old_data = load_csv(old_file)
    new_data = load_csv(new_file)

    shorter = 0
    longer = 0
    same = 0

    sum_short = 0.0
    sum_long = 0.0

    short_list = []
    long_list = []

    for uid, old_time in old_data.items():
        if uid not in new_data:
            continue

        new_time = new_data[uid]
        diff = new_time - old_time

        if diff < 0:
            shorter += 1
            val = -diff
            sum_short += val
            short_list.append(val)
        elif diff > 0:
            longer += 1
            val = diff
            sum_long += val
            long_list.append(val)
        else:
            same += 1

    print("避難時間が短くなったユーザ数:", shorter)
    print("避難時間が長くなったユーザ数:", longer)
    # print("避難時間が変わらないユーザ数:", same)

    print("合計短縮時間:", fmt(sum_short))
    print("合計増加時間:", fmt(sum_long))

    if shorter > 0:
        print("平均短縮時間:", fmt(sum_short / shorter))
        print("最大短縮時間:", fmt(max(short_list)))
        print("最小短縮時間:", fmt(min(short_list)))

    if longer > 0:
        print("平均増加時間:", fmt(sum_long / longer))
        print("最大増加時間:", fmt(max(long_list)))
        print("最小増加時間:", fmt(min(long_list)))


if __name__ == "__main__":
    usernum = "299"
    run = "3"
    old = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/"+usernum+"/Time/EvacuationTime_"+run+".txt"
    new = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/"+usernum+"/Time/EvacuationTime_"+run+".txt"
    compare_times(old, new)
