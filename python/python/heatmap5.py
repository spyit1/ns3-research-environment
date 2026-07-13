import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
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
    
    # 通行止め座標
    if os.path.exists(block_file):
        try:
            df_block = pd.read_csv(block_file)
            for _, row in df_block.iterrows():
                block_coords.append((row.iloc[1], row.iloc[2]))
            print(f"通行止め座標を読み込みました: {len(block_coords)}箇所")
        except Exception as e:
            print(f"通行止めファイルの読み込みエラー: {e}")
    else:
        print(f"警告: ファイルが見つかりません -> {block_file}")

    # 避難所座標
    if os.path.exists(exit_file):
        try:
            df_exit = pd.read_csv(exit_file)
            for _, row in df_exit.iterrows():
                exit_coords.append((row.iloc[1], row.iloc[2]))
            print(f"避難所座標を読み込みました: {len(exit_coords)}箇所")
        except Exception as e:
            print(f"避難所ファイルの読み込みエラー: {e}")
    else:
        print(f"警告: ファイルが見つかりません -> {exit_file}")
        
    return block_coords, exit_coords

# === 個別動画作成関数 ===
def create_marked_movie(config, block_coords, exit_coords, step=10, fps=10):
    file_path = config['path']
    output_filename = config['output']
    label_name = config['label']

    print(f"\n=== 個別動画作成開始: {label_name} ===")
    if not os.path.exists(file_path):
        print(f"  エラー: ファイルなし -> {file_path}")
        return

    try:
        temp = pd.read_csv(file_path, nrows=1)
        target_col = 'exitnode' if 'exitnode' in temp.columns else 'blocknode'
        cols = ['time', 'userID', 'x', 'y', 'AoI', target_col]
        df = pd.read_csv(file_path, usecols=cols)
    except Exception as e:
        print(f"  読み込みエラー: {e}")
        return

    total_info_items = df[target_col].nunique()
    max_time = int(df['time'].max())
    time_steps = list(range(0, max_time + 1, step))

    # グラフ設定
    fig, ax = plt.subplots(figsize=(11, 8))
    
    # 軸範囲決定
    all_x = df['x'].tolist() + [x for x, y in block_coords] + [x for x, y in exit_coords]
    all_y = df['y'].tolist() + [y for x, y in block_coords] + [y for x, y in exit_coords]
    margin = 50
    min_x, max_x = min(all_x) - margin, max(all_x) + margin
    min_y, max_y = min(all_y) - margin, max(all_y) + margin

    # カラーバー
    sc_dummy = ax.scatter([], [], c=[], vmin=0, vmax=total_info_items, cmap='jet')
    cbar = fig.colorbar(sc_dummy, ax=ax)
    cbar.set_label('情報所持数 [個]', fontsize=14)

    def update(frame_time):
        ax.clear()
        # マーカー
        if exit_coords:
            ex_x, ex_y = zip(*exit_coords)
            ax.scatter(ex_x, ex_y, c='green', marker='s', s=120, edgecolors='black', zorder=2, label='避難所')
        if block_coords:
            bl_x, bl_y = zip(*block_coords)
            ax.scatter(bl_x, bl_y, c='red', marker='x', s=120, linewidth=3, zorder=3, label='通行止め')

        # ユーザー
        df_t = df[df['time'] == frame_time].copy()
        if not df_t.empty:
            df_t['is_informed'] = (df_t['AoI'] < df_t['time']).astype(int)
            unique_users = df_t.groupby('userID').agg({'x': 'first', 'y': 'first', 'is_informed': 'sum'}).reset_index()
            ax.scatter(unique_users['x'], unique_users['y'], c=unique_users['is_informed'], 
                       vmin=0, vmax=total_info_items, cmap='jet', alpha=0.7, s=40, edgecolors='gray', linewidth=0.5, zorder=1)
            avg_count = unique_users['is_informed'].mean()
            ax.set_title(f"[{label_name}] Time: {frame_time}s\nAvg Info Count: {avg_count:.2f}", fontsize=16)
        else:
            ax.set_title(f"[{label_name}] Time: {frame_time}s (No Data)", fontsize=16)

        ax.set_xlim(min_x, max_x)
        ax.set_ylim(min_y, max_y)
        ax.legend(loc='upper right', fontsize=10)
        ax.grid(True, linestyle=':', alpha=0.5)
        ax.set_aspect('equal')

    ani = animation.FuncAnimation(fig, update, frames=time_steps, interval=100)
    
    print(f"  保存中... -> {output_filename}")
    if output_filename.endswith('.mp4'):
        try:
            ani.save(output_filename, writer='ffmpeg', fps=fps)
        except:
            ani.save(output_filename.replace('.mp4', '.gif'), writer='pillow', fps=fps)
    else:
        ani.save(output_filename, writer='pillow', fps=fps)
    plt.close()
    print("  完了!")

# === 統合動画作成関数 (4分割画面) ===
def create_combined_movie(configs, block_coords, exit_coords, output_filename='Combined_Movie.mp4', step=10, fps=10):
    print(f"\n=== 統合動画作成開始: {output_filename} ===")

    data_list = []
    global_max_time = 0
    global_info_items = 0
    
    # 軸範囲決定用
    all_x_coords = [x for x, y in block_coords] + [x for x, y in exit_coords]
    all_y_coords = [y for x, y in block_coords] + [y for x, y in exit_coords]

    # データ読み込み
    for config in configs:
        path = config['path']
        label = config['label']
        if not os.path.exists(path):
            print(f"  警告: ファイルなし -> {path}")
            continue
            
        try:
            temp = pd.read_csv(path, nrows=1)
            target_col = 'exitnode' if 'exitnode' in temp.columns else 'blocknode'
            cols = ['time', 'userID', 'x', 'y', 'AoI', target_col]
            df = pd.read_csv(path, usecols=cols)
            
            local_max_time = int(df['time'].max())
            if local_max_time > global_max_time:
                global_max_time = local_max_time
            
            local_info_items = df[target_col].nunique()
            if local_info_items > global_info_items:
                global_info_items = local_info_items
            
            all_x_coords.extend(df['x'].tolist())
            all_y_coords.extend(df['y'].tolist())
            
            data_list.append({'label': label, 'df': df})
            
        except Exception as e:
            print(f"  読み込みエラー ({label}): {e}")

    if not data_list:
        print("  有効なデータがありません。")
        return

    # 軸設定
    margin = 50
    min_x, max_x = min(all_x_coords) - margin, max(all_x_coords) + margin
    min_y, max_y = min(all_y_coords) - margin, max(all_y_coords) + margin

    # プロット準備 (2x2)
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    axes = axes.flatten()
    time_steps = list(range(0, global_max_time + 1, step))

    # 共通カラーバー
    fig.subplots_adjust(right=0.9)
    cbar_ax = fig.add_axes([0.92, 0.15, 0.02, 0.7])
    sc_dummy = axes[0].scatter([], [], c=[], vmin=0, vmax=global_info_items, cmap='jet')
    cbar = fig.colorbar(sc_dummy, cax=cbar_ax)
    cbar.set_label('情報所持数 [個]', fontsize=16)

    def update_combined(frame_time):
        for i, ax in enumerate(axes):
            ax.clear()
            if i >= len(data_list):
                ax.axis('off')
                continue
            
            data = data_list[i]
            df = data['df']
            label = data['label']

            # マーカー
            if exit_coords:
                ex_x, ex_y = zip(*exit_coords)
                ax.scatter(ex_x, ex_y, c='green', marker='s', s=80, edgecolors='black', zorder=2, label='避難所')
            if block_coords:
                bl_x, bl_y = zip(*block_coords)
                ax.scatter(bl_x, bl_y, c='red', marker='x', s=80, linewidth=2, zorder=3, label='通行止め')

            # ユーザー
            df_t = df[df['time'] == frame_time].copy()
            title_text = f"{label}"
            
            if not df_t.empty:
                df_t['is_informed'] = (df_t['AoI'] < df_t['time']).astype(int)
                unique_users = df_t.groupby('userID').agg({'x': 'first', 'y': 'first', 'is_informed': 'sum'}).reset_index()
                ax.scatter(unique_users['x'], unique_users['y'], c=unique_users['is_informed'], 
                           vmin=0, vmax=global_info_items, cmap='jet', alpha=0.7, s=20, edgecolors='none', zorder=1)
                avg = unique_users['is_informed'].mean()
                title_text += f"\nTime: {frame_time}s | Avg: {avg:.2f}"
            else:
                title_text += f"\nTime: {frame_time}s (End)"

            ax.set_title(title_text, fontsize=14)
            ax.set_xlim(min_x, max_x)
            ax.set_ylim(min_y, max_y)
            ax.grid(True, linestyle=':', alpha=0.5)
            ax.set_aspect('equal')
            if i == 0:
                ax.legend(loc='upper right', fontsize=8, framealpha=0.8)

    ani = animation.FuncAnimation(fig, update_combined, frames=time_steps, interval=100)
    
    print(f"  保存中... -> {output_filename}")
    if output_filename.endswith('.mp4'):
        try:
            ani.save(output_filename, writer='ffmpeg', fps=fps)
        except:
            ani.save(output_filename.replace('.mp4', '.gif'), writer='pillow', fps=fps)
    else:
        ani.save(output_filename, writer='pillow', fps=fps)
    plt.close()
    print("  統合動画完了!")


if __name__ == "__main__":
    configure_japanese_font()
    
    # === パス設定 (ユーザー指定) ===
    usernum_str = "499"
    base_dir = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'

    dir_quickhello = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_hello60 = f'{base_dir}/QuickHello_OFF/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_shelter = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'
    dir_road = f'{base_dir}/QuickHello_ON/EraseBlock_OFF/{usernum_str}/AoI_Info/'

    # 座標ファイルパス (ユーザー指定)
    block_xy_file = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/Info/Block/Block_Nodes_xy3.txt'
    exit_xy_file = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/Info/Exit/Exit_Nodes_xy3.txt'

    # ファイル名パターン
    mode = "block"
    filename_pattern = "ALL_Blocked_AoI_Info_{}.txt" if mode == "block" else "ALL_AoI_Info_{}.txt"

    # データセット設定
    dataset_configs = [
        {'label': 'w/o UAV', 'path': dir_quickhello + filename_pattern.format("7"), 'output': 'Movie_Marked_QuickHello_ON.mp4'},
        {'label': 'Hello_60', 'path': dir_hello60 + filename_pattern.format("7"), 'output': 'Movie_Marked_QuickHello_OFF.mp4'},
        {'label': 'ShelterPatrol', 'path': dir_shelter + filename_pattern.format("10"), 'output': 'Movie_Marked_ShelterPatrol.mp4'},
        {'label': 'RoadBlock', 'path': dir_road + filename_pattern.format("20"), 'output': 'Movie_Marked_RoadBlock.mp4'}
    ]

    # === 実行 ===
    # 座標読み込み
    block_coords, exit_coords = load_coordinates(block_xy_file, exit_xy_file)

    # 1. 個別動画作成 (不要ならコメントアウト)
    my_step = 1 
    # for config in dataset_configs:
    #     create_marked_movie(config, block_coords, exit_coords, step=my_step, fps=10)

    # 2. 統合動画作成 (4画面比較)
    create_combined_movie(
        dataset_configs, 
        block_coords, 
        exit_coords, 
        output_filename='Combined_Comparison_Movie.mp4', 
        step=my_step, 
        fps=10
    )