import csv

def load_csv(filename):
    """CSV を userID → time の辞書として読み込む"""
    data = {}
    try:
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
    except FileNotFoundError:
        print(f"エラー: ファイルが見つかりません -> {filename}")
        return {}
    return data

def fmt(x):
    """指数表記を使わず丸める（小数2～4桁）"""
    if x == 0:
        return "0"
    if abs(x) >= 1:
        return f"{x:.2f}"
    else:
        return f"{x:.4f}"

def get_comparison_stats(old_data, new_data):
    """2つのデータを比較して統計情報を辞書で返す"""
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

    # 統計データの構築（リストが空の場合の対策を含む）
    stats = {
        "shorter_count": shorter,
        "longer_count": longer,
        "sum_short": sum_short,
        "sum_long": sum_long,
        
        "avg_short": sum_short / shorter if shorter > 0 else 0,
        "max_short": max(short_list) if shorter > 0 else 0,
        "min_short": min(short_list) if shorter > 0 else 0,
        
        "avg_long": sum_long / longer if longer > 0 else 0,
        "max_long": max(long_list) if longer > 0 else 0,
        "min_long": min(long_list) if longer > 0 else 0,
    }
    return stats

def generate_latex(stats_list):
    """統計リストを受け取りLaTeXの表を出力する"""
    
    # リストの順番: [w/o vs Patrol, w/o vs Focused, Patrol vs Focused]
    s1, s2, s3 = stats_list

    latex_code = f"""
\\begin{{table}}[t]
\\centering
\\caption{{UAV飛行方式の違いによる避難時間への影響}}
\\label{{tab:uav_evacuation_result_{usernum}}}
\\begin{{tabular}}{{lrrr}}
\\hline
% ▼ ここをdiagboxではなく普通の文字にしました ▼
\\makecell{{比較元 \\\\ vs \\\\ 適用方式}} &
\\makecell{{w/o UAV \\\\ vs \\\\ 避難所巡回}} &
\\makecell{{w/o UAV \\\\ vs \\\\ 通行止め重点飛行}} &
\\makecell{{避難所巡回 \\\\ vs \\\\ 通行止め重点飛行}} \\\\
\\hline
避難時間短縮者数 & {s1['shorter_count']} & {s2['shorter_count']} & {s3['shorter_count']} \\\\
避難時間増加者数 & {s1['longer_count']} & {s2['longer_count']} & {s3['longer_count']} \\\\
\\hline
合計短縮時間 [s] & {fmt(s1['sum_short'])} & {fmt(s2['sum_short'])} & {fmt(s3['sum_short'])} \\\\
合計増加時間 [s] & {fmt(s1['sum_long'])} & {fmt(s2['sum_long'])} & {fmt(s3['sum_long'])} \\\\
\\hline
平均短縮時間 [s] & {fmt(s1['avg_short'])} & {fmt(s2['avg_short'])} & {fmt(s3['avg_short'])} \\\\
最大短縮時間 [s] & {fmt(s1['max_short'])} & {fmt(s2['max_short'])} & {fmt(s3['max_short'])} \\\\
最小短縮時間 [s] & {fmt(s1['min_short'])} & {fmt(s2['min_short'])} & {fmt(s3['min_short'])} \\\\
\\hline
平均増加時間 [s] & {fmt(s1['avg_long'])} & {fmt(s2['avg_long'])} & {fmt(s3['avg_long'])} \\\\
最大増加時間 [s] & {fmt(s1['max_long'])} & {fmt(s2['max_long'])} & {fmt(s3['max_long'])} \\\\
最小増加時間 [s] & {fmt(s1['min_long'])} & {fmt(s2['min_long'])} & {fmt(s3['min_long'])} \\\\
\\hline
\\end{{tabular}}
\\end{{table}}
"""
    print(latex_code)

if __name__ == "__main__":
    # === 設定: ここに3つのファイルパスを入力してください ===
    # ファイル1: w/o UAV (ベースライン)
    usernum = "699"
    file_wo_uav = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/"+usernum+"/Time/EvacuationTime_7.txt" 
    
    # ファイル2: 避難所巡回 (Patrol)
    file_patrol = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/"+usernum+"/Time/EvacuationTime_10.txt"
    
    # ファイル3: 通行止め重点飛行 (Focused)
    file_focused = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/"+usernum+"/Time/EvacuationTime_20.txt"
    
    # ==================================================

    # 1. データをロード
    data_wo_uav = load_csv(file_wo_uav)
    data_patrol = load_csv(file_patrol)
    data_focused = load_csv(file_focused)

    if data_wo_uav and data_patrol and data_focused:
        # 2. 比較を実行 (比較元, 適用方式 の順で渡す)
        
        # Case 1: w/o UAV vs 避難所巡回
        stats_1 = get_comparison_stats(old_data=data_wo_uav, new_data=data_patrol)
        
        # Case 2: w/o UAV vs 通行止め重点飛行
        stats_2 = get_comparison_stats(old_data=data_wo_uav, new_data=data_focused)
        
        # Case 3: 避難所巡回 vs 通行止め重点飛行
        # (表のヘッダー順に従い、左が比較元、右が適用方式と仮定)
        stats_3 = get_comparison_stats(old_data=data_patrol, new_data=data_focused)

        # 3. LaTeXテーブルを出力
        generate_latex([stats_1, stats_2, stats_3])
    else:
        print("データの読み込みに失敗したため、処理を中断しました。")