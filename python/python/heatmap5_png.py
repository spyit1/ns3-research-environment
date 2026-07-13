import pandas as pd
import matplotlib.pyplot as plt
import os
import matplotlib.font_manager as fm
import numpy as np

# === フォント設定 ===
def configure_japanese_font():
    font_candidates = ['Noto Sans CJK JP', 'IPAGothic', 'TakaoGothic', 'VL Gothic', 'Hiragino Sans', 'Meiryo', 'MS Gothic', 'AppleGothic']
    plt.rcParams['font.family'] = 'sans-serif'
    plt.rcParams['font.sans-serif'] = font_candidates + plt.rcParams['font.sans-serif']
    plt.rcParams['axes.unicode_minus'] = False

# === 座標データの読み込み ===
def load_coordinates(block_file, exit_file):
    block_coords = []
    exit_coords = []
    
    if os.path.exists(block_file):
        try:
            df_block = pd.read_csv(block_file)
            for _, row in df_block.iterrows():
                block_coords.append((row.iloc[1], row.iloc[2]))
            print(f"通行止め座標: {len(block_coords)}箇所")
        except Exception as e:
            print(f"通行止めファイルエラー: {e}")

    if os.path.exists(exit_file):
        try:
            df_exit = pd.read_csv(exit_file)
            for _, row in df_exit.iterrows():
                exit_coords.append((row.iloc[1], row.iloc[2]))
            print(f"避難所座標: {len(exit_coords)}箇所")
        except Exception as e:
            print(f"避難所ファイルエラー: {e}")
        
    return block_coords, exit_coords

# === ★★★ 指定時刻のスナップショットを作成する関数 ★★★ ===
def save_combined_snapshot_at_time(configs, block_coords, exit_coords, target_time, output_filename, font_sizes):
    print(f"\n=== スナップショット作成開始: 時刻 {target_time}秒 ===")

    # 1. 全データの読み込みと範囲決定
    data_list = []
    global_info_items = 0
    
    all_x_coords = [x for x, y in block_coords] + [x for x, y in exit_coords]
    all_y_coords = [y for x, y in block_coords] + [y for x, y in exit_coords]

    for config in configs:
        path = config['path']
        label = config['label']
        
        if not os.path.exists(path):
            print(f"  警告: ファイルなし -> {path}")
            data_list.append({'label': label, 'df': pd.DataFrame(), 'valid': False})
            continue
            
        try:
            temp = pd.read_csv(path, nrows=1)
            target_col = 'exitnode' if 'exitnode' in temp.columns else 'blocknode'
            cols = ['time', 'userID', 'x', 'y', 'AoI', target_col]
            df = pd.read_csv(path, usecols=cols)
            
            local_info_items = df[target_col].nunique()
            if local_info_items > global_info_items:
                global_info_items = local_info_items
            
            all_x_coords.extend(df['x'].tolist())
            all_y_coords.extend(df['y'].tolist())
            
            data_list.append({'label': label, 'df': df, 'target_col': target_col, 'valid': True})
            
        except Exception as e:
            print(f"  読み込みエラー ({label}): {e}")
            data_list.append({'label': label, 'df': pd.DataFrame(), 'valid': False})

    if not all_x_coords:
        print("  有効な座標データがありません。")
        return

    # 軸範囲の設定
    margin = 50
    min_x, max_x = min(all_x_coords) - margin, max(all_x_coords) + margin
    min_y, max_y = min(all_y_coords) - margin, max(all_y_coords) + margin

    # 2. プロット作成
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    axes = axes.flatten()

    # レイアウト調整 (余白を詰める)
    fig.subplots_adjust(left=0.05, bottom=0.05, right=0.8, top=0.95, wspace=0.1, hspace=0.2)

    # 共通カラーバーの設定
    cbar_ax = fig.add_axes([0.81, 0.15, 0.02, 0.7])
    sc_dummy = axes[0].scatter([], [], c=[], vmin=0, vmax=global_info_items, cmap='jet')
    cbar = fig.colorbar(sc_dummy, cax=cbar_ax)
    
    # ★文字サイズ適用: カラーバー
    cbar.set_label('情報所持数 [個]', fontsize=font_sizes['colorbar_label'])
    cbar.ax.tick_params(labelsize=font_sizes['tick_label'])

    # 各サブプロットの描画
    for i, ax in enumerate(axes):
        if i >= len(data_list):
            ax.axis('off')
            continue
        
        data = data_list[i]
        label = data['label']
        
        # --- 背景マーカー ---
        if exit_coords:
            ex_x, ex_y = zip(*exit_coords)
            ax.scatter(ex_x, ex_y, c='green', marker='s', s=80, edgecolors='black', zorder=2, label='避難所')
        if block_coords:
            bl_x, bl_y = zip(*block_coords)
            ax.scatter(bl_x, bl_y, c='red', marker='x', s=80, linewidth=2, zorder=3, label='通行止め')

        # --- ユーザーデータの描画 ---
        title_text = f"{label}"
        if data['valid']:
            df = data['df']
            df_t = df[df['time'] == target_time].copy()
            
            if not df_t.empty:
                df_t['is_informed'] = (df_t['AoI'] < df_t['time']).astype(int)
                unique_users = df_t.groupby('userID').agg({'x': 'first', 'y': 'first', 'is_informed': 'sum'}).reset_index()
                
                ax.scatter(unique_users['x'], unique_users['y'], c=unique_users['is_informed'], 
                           vmin=0, vmax=global_info_items, cmap='jet', 
                           alpha=0.7, s=30, edgecolors='none', zorder=1)
                
                avg = unique_users['is_informed'].mean()
                title_text += f"\nTime: {target_time}s | ユーザ一人あたりの平均情報所持数: {avg:.2f}"
            else:
                title_text += f"\nTime: {target_time}s (No Data)"
        else:
            title_text += " (Load Error)"

        # ★文字サイズ適用: タイトルと軸メモリ
        ax.set_title(title_text, fontsize=font_sizes['title'])
        ax.tick_params(labelsize=font_sizes['tick_label'])
        
        ax.set_xlim(min_x, max_x)
        ax.set_ylim(min_y, max_y)
        ax.grid(True, linestyle=':', alpha=0.5)
        ax.set_aspect('equal')
        
        # ★文字サイズ適用: 凡例 (左上のみ)
        if i == 0:
            ax.legend(loc='upper right', fontsize=font_sizes['legend'], framealpha=0.8)

    # 保存
    plt.savefig(output_filename, bbox_inches='tight')
    plt.close()
    print(f"  -> 保存完了: {output_filename}")


if __name__ == "__main__":
    configure_japanese_font()
    
    # === 設定エリア ===
    TARGET_TIME = 300
    
    # ★★★ ここで文字サイズを一括設定できます ★★★
    FONT_CONFIG = {
        'title': 18,           # 各グラフのタイトル (Avgなどの文字)
        'tick_label': 12,      # 軸の数値 (1000, 2000など) とカラーバーの目盛り
        'legend': 18,          # 凡例 (避難所・通行止めの説明)
        'colorbar_label': 18   # カラーバーのタイトル「情報所持数 [個]」
    }

    usernum_str = "699"
    base_dir = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'

    dir_quickhello = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_hello60 = f'{base_dir}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_shelter = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_road = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'

    # 座標ファイルパス
    block_xy_file = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/Info/Block/Block_Nodes_xy3.txt'
    exit_xy_file = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/Info/Exit/Exit_Nodes_xy3.txt'

    # ファイル名パターン
    mode = "block"
    filename_pattern = "ALL_Blocked_AoI_Info_{}.txt" if mode == "block" else "ALL_AoI_Info_{}.txt"

    dataset_configs = [
        {'label': 'w/o UAV（QuickHello）', 'path': dir_quickhello + filename_pattern.format("7")},
        {'label': 'Hello=60', 'path': dir_hello60 + filename_pattern.format("7")},
        {'label': '避難所巡回', 'path': dir_shelter + filename_pattern.format("10")},
        {'label': '通行止め重点飛行', 'path': dir_road + filename_pattern.format("20")}
    ]

    # === 実行 ===
    block_coords, exit_coords = load_coordinates(block_xy_file, exit_xy_file)

    output_name = f'Snapshot_Time_{TARGET_TIME}s.png'
    
    save_combined_snapshot_at_time(
        dataset_configs, 
        block_coords, 
        exit_coords, 
        target_time=TARGET_TIME, 
        output_filename=output_name,
        font_sizes=FONT_CONFIG  # フォント設定を渡す
    )