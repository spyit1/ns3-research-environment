import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import matplotlib.font_manager as fm

def configure_japanese_font():
    """
    Matplotlibで日本語を表示するための設定を行う関数
    """
    font_candidates = [
        'Noto Sans CJK JP', 
        'IPAGothic', 
        'TakaoGothic', 
        'VL Gothic', 
        'Hiragino Sans', 
        'Meiryo', 
        'MS Gothic',
        'AppleGothic'
    ]
    plt.rcParams['font.family'] = 'sans-serif'
    plt.rcParams['font.sans-serif'] = font_candidates + plt.rcParams['font.sans-serif']
    plt.rcParams['axes.unicode_minus'] = False

def plot_average_coverage_ratio(file_on, file_off, total_users, target_col_name='exitnode', y_limit=105, fontsize=14):
    """
    全ターゲットの所持率平均をグラフ化する。
    
    Args:
        y_limit (float): Y軸の最大値（Noneなら自動）
        fontsize (int): グラフ内の文字サイズ
    """
    print(f"--- [まとめ処理] 開始: 対象カラム = {target_col_name} ---")

    # 1. データ読み込み
    try:
        temp_df = pd.read_csv(file_on, nrows=1)
        if target_col_name not in temp_df.columns:
            print(f"エラー: 列 '{target_col_name}' がありません。")
            return

        cols = ['time', 'userID', 'AoI', target_col_name]
        df_on = pd.read_csv(file_on, usecols=cols)
        df_off = pd.read_csv(file_off, usecols=cols)
    except Exception as e:
        print(f"ファイル読み込みエラー: {e}")
        return

    # 全ターゲットID取得
    target_ids = df_on[target_col_name].unique()
    print(f"集計対象ID数: {len(target_ids)} 個, 全ユーザー数: {total_users}")

    # --- 内部関数 ---
    def get_coverage_series(df_data, target_id, total_u, global_max_time):
        sub_data = df_data[df_data[target_col_name] == target_id]
        if sub_data.empty: return None

        informed_df = sub_data[sub_data['AoI'] < sub_data['time']].copy()
        if informed_df.empty:
            return pd.Series(0.0, index=range(global_max_time + 1))

        informed_df = informed_df.sort_values('time')
        first_informed_time = informed_df.groupby('userID')['time'].min()
        
        new_informed_counts = first_informed_time.value_counts().sort_index()
        accumulated_counts = new_informed_counts.cumsum()
        
        ratio_series = (accumulated_counts / total_u) * 100
        
        full_index = range(global_max_time + 1)
        aligned_series = ratio_series.reindex(full_index, method='ffill').fillna(0.0)
        
        return aligned_series

    # --- メイン処理 ---
    max_time_on = int(df_on['time'].max())
    max_time_off = int(df_off['time'].max())
    global_max_time = max(max_time_on, max_time_off)

    list_series_on = []
    list_series_off = []

    print("各拠点のデータを集計中...")
    
    for tid in target_ids:
        s_on = get_coverage_series(df_on, tid, total_users, global_max_time)
        if s_on is not None: list_series_on.append(s_on)
            
        s_off = get_coverage_series(df_off, tid, total_users, global_max_time)
        if s_off is not None: list_series_off.append(s_off)

    if not list_series_on or not list_series_off:
        print("有効なデータが集まりませんでした。")
        return

    df_res_on = pd.concat(list_series_on, axis=1)
    df_res_off = pd.concat(list_series_off, axis=1)

    mean_on = df_res_on.mean(axis=1)
    mean_off = df_res_off.mean(axis=1)

    # === グラフ描画 ===
    plt.figure(figsize=(10, 6))

    plt.plot(mean_on.index, mean_on.values, label='Quick Hello', color='orange', linewidth=2)
    plt.plot(mean_off.index, mean_off.values, label='Hello=60', color='blue', linestyle='--', linewidth=2)

    # ★文字サイズの設定を適用
    plt.xlabel('シミュレーション時間 [s]', fontsize=fontsize)
    plt.ylabel('情報所持率（平均） [%]', fontsize=fontsize)
    
    # ★Y軸の最大値設定
    if y_limit is not None:
        plt.ylim(0, y_limit)
    else:
        plt.ylim(bottom=0)

    # 目盛り（数字）の文字サイズ
    plt.tick_params(labelsize=fontsize)

    # 凡例の文字サイズ
    plt.legend(loc='lower right', fontsize=fontsize)
    
    plt.grid(True)

    output_file = f'Average_Coverage_Ratio_{target_col_name}.png'
    plt.savefig(output_file)
    plt.close()
    print(f"-> Saved Summary Graph: {output_file}")


if __name__ == "__main__":
    configure_japanese_font()

    # === 設定 ===
    usernum_str = "299"
    run = "3"
    total_users_count = int(usernum_str) + 1
    base_path = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'
    
    # ==========================================
    # ★★★ ここでグラフの見た目を調整してください ★★★
    # ==========================================
    
    # 文字の大きさ（デフォルト: 14, 論文用なら 16~18 推奨）
    my_fontsize = 18
    
    # Y軸の最大値（None にするとデータに合わせて自動調整）
    # 例: 100 にするとぴったり100%まで。105 だと少し余裕ができる。
    my_y_limit = None
    
    # モード切り替え ("block" または "shelter")
    mode = "block"   

    # ==========================================

    if mode == "block":
        col_name = "blocknode"
        file_on = f'{base_path}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/ALL_Blocked_AoI_Info_{run}.txt'
        file_off = f'{base_path}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/ALL_Blocked_AoI_Info_{run}.txt'

    elif mode == "shelter":
        col_name = "exitnode"
        file_on = f'{base_path}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/ALL_AoI_Info_{run}.txt'
        file_off = f'{base_path}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/ALL_AoI_Info_{run}.txt'

    if not os.path.exists(file_on):
        print(f"エラー: ファイルが見つかりません -> {file_on}")
    else:
        plot_average_coverage_ratio(
            file_on, file_off, total_users_count, 
            target_col_name=col_name, 
            y_limit=my_y_limit,       # ★追加した引数
            fontsize=my_fontsize      # ★追加した引数
        )