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

def plot_average_coverage_ratio(dataset_configs, total_users, target_col_name='exitnode', y_limit=105, fontsize=14):
    """
    複数のデータセットの所持率平均を1枚のグラフにする。
    """
    print(f"--- [まとめ処理] 開始: 対象カラム = {target_col_name} ---")

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

    # === 1. 各ファイルの読み込みと集計 ===
    global_max_time = 0
    loaded_dfs = []
    valid_configs = []

    for config in dataset_configs:
        file_path = config['path']
        label = config['label']
        
        if not os.path.exists(file_path):
            print(f"警告: ファイルが見つかりません -> {file_path}")
            continue
            
        try:
            # ヘッダーチェック
            temp_df = pd.read_csv(file_path, nrows=1)
            if target_col_name not in temp_df.columns:
                print(f"エラー: 列 '{target_col_name}' が {label} のファイルにありません。スキップします。")
                continue

            cols = ['time', 'userID', 'AoI', target_col_name]
            df = pd.read_csv(file_path, usecols=cols)
            
            max_t = int(df['time'].max())
            if max_t > global_max_time:
                global_max_time = max_t
            
            loaded_dfs.append(df)
            valid_configs.append(config)
            
        except Exception as e:
            print(f"{label} の読み込みエラー: {e}")

    if not loaded_dfs:
        print("有効なデータが1つもありませんでした。終了します。")
        return

    print(f"全データの最大時間: {global_max_time}秒")

    # === 2. 各データセットごとの平均計算 ===
    plt.figure(figsize=(10, 7)) # 凡例が上に来る分、縦を少し伸ばしても良いかも

    for df, config in zip(loaded_dfs, valid_configs):
        label = config['label']
        print(f"集計中: {label} ...")
        
        target_ids = df[target_col_name].unique()
        list_series = []

        for tid in target_ids:
            s = get_coverage_series(df, tid, total_users, global_max_time)
            if s is not None:
                list_series.append(s)
        
        if list_series:
            df_res = pd.concat(list_series, axis=1)
            mean_val = df_res.mean(axis=1)
            
            # プロット
            plt.plot(mean_val.index, mean_val.values, 
                     label=label, 
                     color=config['color'], 
                     linestyle=config['linestyle'], 
                     linewidth=2)
        else:
            print(f"  -> {label} は有効なデータ系列が生成されませんでした。")

    # === 3. グラフの体裁設定 ===
    plt.xlabel('シミュレーション時間 [s]', fontsize=fontsize)
    plt.ylabel('情報所持率（平均） [%]', fontsize=fontsize)
    
    if y_limit is not None:
        plt.ylim(0, y_limit)
    else:
        plt.ylim(bottom=0)

    plt.tick_params(labelsize=fontsize)
    plt.grid(True)

    # ★★★ 変更箇所: 凡例を上部に2段(2列)で配置 ★★★
    # ncol=2 にすることで、4つのデータが「2列×2行」で表示されます
    plt.legend(
        bbox_to_anchor=(0.5, 1.01), # グラフ上端の中央
        loc='lower center',         # 凡例ボックスの下中央を基準点に合わせる
        borderaxespad=0, 
        fontsize=fontsize,
        ncol=3                      # ここで列数（＝2列にすれば4項目なので2段になる）を指定
    )

    # レイアウト調整
    plt.tight_layout()

    output_file = f'Average_Coverage_Comparison_{target_col_name}_{total_users_count}.png'
    
    # 保存時に凡例が見切れないようにする
    plt.savefig(output_file, bbox_inches='tight')
    
    plt.close()
    print(f"-> Saved Summary Graph: {output_file}")


if __name__ == "__main__":
    configure_japanese_font()

    # === 設定 ===
    usernum_str = "499"
    total_users_count = int(usernum_str) + 1
    
    # 基本パス (環境に合わせて変更してください)
    base_dir = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'
    
    # 各手法のディレクトリパス
    dir_quickhello = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_hello60 = f'{base_dir}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_shelter_patrol = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/' 
    dir_road_block = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/' 

    # === モード切り替え ("block" or "shelter") ===
    mode = "block"
    # mode = "shelter"

    if mode == "block":
        col_name = "blocknode"
    elif mode == "shelter":
        col_name = "exitnode"

    def get_log_filename(mode_name, run_num):
        if mode_name == "block":
            return f'ALL_Blocked_AoI_Info_{run_num}.txt'
        elif mode_name == "shelter":
            return f'ALL_AoI_Info_{run_num}.txt'
        return ""

    # === データセットの設定リスト ===
    dataset_configs = [
        {
            'label': 'UAVなし',
            'path': dir_quickhello + get_log_filename(mode, "5"),
            'color': 'orange',
            'linestyle': '-'
        },
        {
            'label': 'Hello=60',
            'path': dir_hello60 + get_log_filename(mode, "300000"),
            'color': 'blue',
            'linestyle': '--'
        },
        {
            'label': '避難所巡回方式',
            'path': dir_shelter_patrol + get_log_filename(mode, "10"),
            'color': 'green',
            'linestyle': '-.'
        },
        {
            'label': '通行止め重点飛行方式',
            'path': dir_road_block + get_log_filename(mode, "20"),
            'color': 'red',
            'linestyle': ':'
        }
    ]

    # === 実行 ===
    my_fontsize = 18
    my_y_limit = None # データに合わせて自動調整

    plot_average_coverage_ratio(
        dataset_configs, 
        total_users_count, 
        target_col_name=col_name, 
        y_limit=my_y_limit,
        fontsize=my_fontsize
    )