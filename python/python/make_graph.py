import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.colors as mcolors
import os

# ==========================================
# 0. 設定項目 (ここを環境に合わせて変更してください)
# ==========================================

# 出力ディレクトリ
OUTPUT_DIRECTORY = "/home/obayashi/ns-allinone-3.35/ns-3.35/heatmap/block_diff_video_strict"
os.makedirs(OUTPUT_DIRECTORY, exist_ok=True)

# 【重要】比較する2つのログファイルパス
# FILE_PROPOSED: 提案手法（UAV重点飛行） -> これを基準にします (末尾 _3.txt)
FILE_PROPOSED = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_3.txt"

# FILE_COMPARISON: 比較対象の手法
#  - UAVなしと比較したい場合: "..._0.txt" を指定してください (青色が出やすい)
#  - 避難所巡回と比較したい場合: "..._1.txt" を指定してください (赤色ばかりになる)
FILE_COMPARISON = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_1.txt"

# 通行止めノードの座標リストファイル
BLOCK_LOCATIONS_PATH = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/Info/Block/Block_Nodes_xy3.txt"

# カラーマップの範囲 (差分の最大・最小)
# 赤(提案勝ち: AoIが小さい) <--- 0 ---> 青(比較対象勝ち: AoIが小さい)
DIFF_VMAX = 300  
DIFF_VMIN = -300

# =========================================================

print(f"動画出力先: {OUTPUT_DIRECTORY}")
print(f"提案手法ログ: {os.path.basename(FILE_PROPOSED)}")
print(f"比較対象ログ: {os.path.basename(FILE_COMPARISON)}")

# ====== 1. データの読み込み ======
print("Loading log files...")
try:
    # 必要なカラムを読み込み
    # ★重要: 両方のファイルから 'blocknode' を読み込みます
    use_cols = ['time', 'userID', 'x', 'y', 'AoI', 'blocknode']
    
    df_p = pd.read_csv(FILE_PROPOSED, usecols=use_cols)
    df_c = pd.read_csv(FILE_COMPARISON, usecols=use_cols)
    
    # 座標ファイルの読み込み
    block_locations_df = pd.read_csv(BLOCK_LOCATIONS_PATH)
    block_locations_df.columns = block_locations_df.columns.str.strip()
    
    # カラム名変更 (結合後の識別用)
    # 座標(x,y)は提案手法のもの(x_p, y_p)を使用します
    df_p = df_p.rename(columns={'AoI': 'AoI_prop', 'x': 'x_p', 'y': 'y_p'})
    df_c = df_c.rename(columns={'AoI': 'AoI_comp'}) # comp = comparison
    
except FileNotFoundError as e:
    print(f"Error: file not found. {e}")
    exit()

# ====== 2. 描画範囲の設定 (提案手法のログ全体を基準) ======
x_min_global, x_max_global = df_p['x_p'].min(), df_p['x_p'].max()
y_min_global, y_max_global = df_p['y_p'].min(), df_p['y_p'].max()

# ====== y 座標反転関数 ======
def invert_y(y):
    return y_max_global + y_min_global - y

# ====== 3. カラーマップ設定 ======
# 赤 = プラス (提案手法が良い / AoI差分が正)
# 青 = マイナス (比較手法が良い / AoI差分が負)
cmap = plt.get_cmap('RdBu_r') 
norm = mcolors.TwoSlopeNorm(vmin=DIFF_VMIN, vcenter=0., vmax=DIFF_VMAX)

# ====== 4. 通行止めノードごとの動画作成 ======
# ログに含まれるユニークなblocknodeを取得
unique_blocks = df_p['blocknode'].astype(str).str.strip().unique()

print(f"Found {len(unique_blocks)} blocked nodes.")

for block in unique_blocks:
    print(f"--- Processing Block Node: {block} ---")

    # 1. 提案手法データから「この通行止め地点」の情報を抽出
    sub_p = df_p[df_p['blocknode'].astype(str).str.strip() == block]
    
    # 2. 比較対象データからも「この通行止め地点」の情報を抽出
    # ★ここが重要: 比較対象が「違う地点の情報」を持っていた場合、それは除外されます
    sub_c = df_c[df_c['blocknode'].astype(str).str.strip() == block]
    
    if sub_p.empty or sub_c.empty:
        print(f"  -> Skipping {block} (Insufficient data in one of the files)")
        continue

    # 3. 時刻とユーザIDで結合
    # これにより「同じ時刻、同じユーザ、同じ通行止め地点の情報」同士が横並びになります
    df_merged = pd.merge(
        sub_p[['time', 'userID', 'x_p', 'y_p', 'AoI_prop']],
        sub_c[['time', 'userID', 'AoI_comp']],
        on=['time', 'userID'],
        how='inner'
    )
    
    # 差分計算: (比較対象AoI) - (提案AoI)
    # プラス = 比較対象の方が古い = 提案の方が新しい = 赤色
    df_merged['Diff'] = df_merged['AoI_comp'] - df_merged['AoI_prop']

    # この通行止めノードの座標を取得
    target_block_info = block_locations_df[block_locations_df['blocknodeid'].astype(str).str.strip() == block]
    
    # 時間リスト
    t_vals = sorted(df_merged["time"].unique())

    # --- 図の準備 ---
    fig, ax = plt.subplots(figsize=(10, 8))
    
    # カラーバー設定
    mappable = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
    cbar = fig.colorbar(mappable, ax=ax)
    cbar.set_label("AoI Difference [s]\n(Red: Proposed is Better, Blue: Comparison is Better)")

    def update_block_video(frame):
        ax.clear()
        current_time = t_vals[frame]
        
        # その時刻のデータ
        df_t = df_merged[df_merged["time"] == current_time]

        # 1. 通行止め位置プロット (赤いバツ印)
        if not target_block_info.empty:
            ax.scatter(
                target_block_info['x'].iloc[0],
                invert_y(target_block_info['y'].iloc[0]),
                marker='X', color='red', s=400, linewidths=3, edgecolors='black', zorder=10, label='Blockage'
            )
            
        # 2. ユーザプロット (色は差分)
        if not df_t.empty:
            ax.scatter(
                df_t['x_p'],
                invert_y(df_t['y_p']),
                c=df_t['Diff'],
                cmap=cmap, norm=norm, s=40, alpha=0.9, edgecolors='grey', linewidth=0.3, zorder=5
            )

        ax.set_title(f"AoI Difference - Blockage: {block}\n(Time={current_time:.2f} s)")
        ax.set_xlabel("X (m)")
        ax.set_ylabel("Y (m)")
        ax.set_xlim(x_min_global, x_max_global)
        ax.set_ylim(y_min_global, y_max_global)
        ax.set_aspect('equal', adjustable='box')
        ax.grid(True, linestyle='--', alpha=0.3)
        ax.legend(loc='upper right')

    # アニメーション作成
    ani = animation.FuncAnimation(fig, update_block_video, frames=len(t_vals), interval=100) # intervalを100msに短縮

    # ファイル名に使えない文字を置換
    safe_name = block.replace('/', '_')
    output_filename = f"aoi_diff_strict_{safe_name}.mp4"
    
    ani.save(os.path.join(OUTPUT_DIRECTORY, output_filename), writer="ffmpeg", dpi=150)
    plt.close(fig)

    print(f"  -> Saved {output_filename}")

print("\nAll processing completed.")