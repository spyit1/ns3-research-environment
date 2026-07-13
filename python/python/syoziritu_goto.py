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

def plot_accumulated_coverage_ratio_multi(dataset_configs, total_users, target_col_name='blocknode', target_nodes=None, y_limit=105, fontsize=14):
    """
    シミュレーション時間と情報所持率（Coverage Ratio）の関係をグラフ化する。
    複数のデータセットを比較可能。
    
    Args:
        dataset_configs (list): 設定辞書のリスト (path, label, color, linestyle)
        total_users (int): 全ユーザー数（分母）
        target_col_name (str): ターゲット識別子のカラム名 ('blocknode' や 'exitnode')
    """
    print(f"--- 処理開始: 対象カラム = {target_col_name} ---")

    # === 1. 全データの読み込み ===
    loaded_data = [] # (dataframe, config) のタプルを格納
    all_target_ids = set()

    for config in dataset_configs:
        file_path = config['path']
        label = config['label']
        
        if not os.path.exists(file_path):
            print(f"警告: ファイルが見つかりません -> {file_path}")
            continue
            
        try:
            # カラム存在確認
            temp_df = pd.read_csv(file_path, nrows=1)
            if target_col_name not in temp_df.columns:
                print(f"エラー: 列 '{target_col_name}' が {label} のファイルにありません。")
                continue

            # 読み込み
            cols = ['time', 'userID', 'AoI', target_col_name]
            df = pd.read_csv(file_path, usecols=cols)
            loaded_data.append((df, config))
            
            # ターゲットID（通行止め箇所など）の収集
            ids = df[target_col_name].unique()
            all_target_ids.update(ids)
            
        except Exception as e:
            print(f"{label} の読み込みエラー: {e}")

    if not loaded_data:
        print("有効なデータがありません。終了します。")
        return

    # 対象ノードの決定
    if target_nodes is None:
        target_nodes = sorted(list(all_target_ids))
    
    print(f"全ユーザー数（分母）: {total_users}")
    print(f"処理対象ノード数: {len(target_nodes)}")

    # === 2. 各ノードごとにグラフ作成 ===
    for target in target_nodes:
        print(f"Processing accumulated coverage for {target}...")
        
        # プロット準備
        plt.figure(figsize=(10, 6))
        has_data = False

        # データセットごとにループ
        for df, config in loaded_data:
            # そのターゲットのデータのみ抽出
            sub_data = df[df[target_col_name] == target]
            
            if sub_data.empty:
                continue

            # --- 累積所持率計算ロジック ---
            informed_df = sub_data[sub_data['AoI'] < sub_data['time']].copy()
            
            if informed_df.empty:
                # データはあるが所持者がいない場合、0%の線を引くか、スキップするか
                # ここではスキップせず0%として描画した方が比較しやすいが、
                # 元コードの挙動に合わせて空なら描画しない実装にします
                continue

            has_data = True
            
            # 時刻でソート
            informed_df = informed_df.sort_values('time')
            
            # 初回取得時刻
            first_informed_time = informed_df.groupby('userID')['time'].min()
            
            # 累積和
            new_informed_counts = first_informed_time.value_counts().sort_index()
            accumulated_counts = new_informed_counts.cumsum()
            
            # 最後まで値を維持する処理 (前方埋め)
            max_time = df['time'].max() # ファイル全体の最大時間を使う
            if accumulated_counts.index.max() < max_time:
                accumulated_counts[max_time] = accumulated_counts.iloc[-1]
            
            # 割合(%)
            ratio_series = (accumulated_counts / total_users) * 100
            
            # 0秒地点を0%にする（グラフの開始位置調整）
            ratio_series[0] = 0.0
            ratio_series = ratio_series.sort_index()

            # プロット
            plt.plot(ratio_series.index, ratio_series.values, 
                     label=config['label'], 
                     color=config['color'], 
                     linestyle=config['linestyle'], 
                     linewidth=2)

        if not has_data:
            plt.close()
            continue

        # === グラフの体裁 ===
        plt.title(f'Accumulated Information Coverage Ratio\nTarget: {target}', fontsize=fontsize)
        plt.xlabel('シミュレーション時間 [s]', fontsize=fontsize)
        plt.ylabel('累積情報所持率 [%]', fontsize=fontsize)
        
        if y_limit is not None:
            plt.ylim(0, y_limit)
        else:
            plt.ylim(bottom=0)

        plt.tick_params(labelsize=fontsize)
        plt.grid(True)
        
        # 凡例の設定（右外に配置）
        plt.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0, fontsize=fontsize)
        
        # 保存
        safe_name = str(target).replace('/', '_').replace(':', '')
        output_file = f'Acc_Coverage_{safe_name}.png'
        plt.tight_layout() # 一旦レイアウト調整
        plt.savefig(output_file, bbox_inches='tight') # 凡例が見切れないように保存
        plt.close()
        print(f"-> Saved: {output_file}")


if __name__ == "__main__":
    configure_japanese_font()

    # === 設定 ===
    usernum_str = "699"
    total_users_count = 700
    
    # 基本パス
    base_dir = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'

    # 各ディレクトリ
    # ※実際のディレクトリ名に合わせて修正してください
    dir_quickhello = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_hello60 = f'{base_dir}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_shelter_patrol = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/' # 例
    dir_road_block = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'     # 例

    # === モード切り替え ("block" or "shelter") ===
    mode = "block"
    
    if mode == "block":
        col_name = "blocknode"
    elif mode == "shelter":
        col_name = "exitnode"

    # ファイル名生成関数
    def get_log_filename(mode_name, run_num):
        if mode_name == "block":
            return f'ALL_Blocked_AoI_Info_{run_num}.txt'
        elif mode_name == "shelter":
            return f'ALL_AoI_Info_{run_num}.txt'
        return ""

    # === データセットの設定リスト ===
    # ここで4つのファイルを定義します
    dataset_configs = [
        {
            'label': 'w/o UAV',  # QuickHello ON
            'path': dir_quickhello + get_log_filename(mode, "7"), # run 7
            'color': 'orange',
            'linestyle': '-'
        },
        {
            'label': 'Hello=60', # QuickHello OFF
            'path': dir_hello60 + get_log_filename(mode, "7"),    # run 7
            'color': 'blue',
            'linestyle': '--'
        },
        {
            'label': '避難所巡回',
            'path': dir_shelter_patrol + get_log_filename(mode, "10"), # run 10 (例)
            'color': 'green',
            'linestyle': '-.'
        },
        {
            'label': '通行止め重点飛行方式',
            'path': dir_road_block + get_log_filename(mode, "20"),     # run 20 (例)
            'color': 'red',
            'linestyle': ':'
        }
    ]

    # === 実行 ===
    my_fontsize = 16
    my_y_limit = 100

    plot_accumulated_coverage_ratio_multi(
        dataset_configs, 
        total_users_count, 
        target_col_name=col_name, 
        y_limit=my_y_limit,
        fontsize=my_fontsize
    )