import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import platform

# ==========================================
# 0. 日本語フォントの設定
# ==========================================
system_name = platform.system()
if system_name == 'Windows':
    plt.rcParams['font.family'] = 'MS Gothic'
elif system_name == 'Darwin': # Mac
    plt.rcParams['font.family'] = 'Hiragino Sans'
else:
    plt.rcParams['font.family'] = 'IPAGothic'
    try:
        import matplotlib.font_manager as fm
        fonts = [f.name for f in fm.fontManager.ttflist]
        if 'IPAGothic' not in fonts and 'TakaoGothic' in fonts:
            plt.rcParams['font.family'] = 'TakaoGothic'
    except:
        pass

# ==========================================
# 1. 設定項目
# ==========================================
# ファイルパス
FILE_EXISTING = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/AoI_Info/ALL_Blocked_AoI_Info_1.txt" # UAVなし
FILE_PATROL   = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/AoI_Info/ALL_Blocked_AoI_Info_2.txt" # 避難所巡回 (追加)
FILE_PROPOSED = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/AoI_Info/ALL_Blocked_AoI_Info_3.txt" # 通行止め重点 (提案)

# 出力先フォルダ
OUTPUT_DIR = 'graph_output_jp_final'
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

# ==========================================
# 2. データ読み込みと前処理
# ==========================================
print("データを読み込み中...")
cols = ['time', 'userID', 'blocknode', 'AoI']

def load_and_process(filepath):
    if not os.path.exists(filepath):
        print(f"エラー: ファイルが見つかりません -> {filepath}")
        return None
    try:
        df = pd.read_csv(filepath, usecols=cols)
    except Exception as e:
        print(f"読み込みエラー: {e}")
        return None

    if df['blocknode'].dtype == object:
        df['blocknode'] = df['blocknode'].astype(str).str.strip()

    # 有効な情報受信のみ抽出 (time > AoI + 1.0)
    valid_df = df[df['time'] > (df['AoI'] + 1.0)].copy()

    # 初回受信時刻の取得
    first_times = valid_df.groupby(['userID', 'blocknode'])['time'].min().reset_index()
    return first_times

# 3つのファイルをロード
t_ex = load_and_process(FILE_EXISTING)
t_pt = load_and_process(FILE_PATROL)   # 追加: 避難所巡回
t_pr = load_and_process(FILE_PROPOSED)

if t_ex is None or t_pr is None or t_pt is None:
    print("データ不足のため終了します。")
    exit()

# カラム名変更
t_ex = t_ex.rename(columns={'time': 'time_existing'})
t_pt = t_pt.rename(columns={'time': 'time_patrol'}) # 追加
t_pr = t_pr.rename(columns={'time': 'time_proposed'})

# 散布図用に「既存 vs 提案」だけマージ（以前のロジック維持）
merged = pd.merge(t_ex, t_pr, on=['userID', 'blocknode'], how='outer')

# ==========================================
# 3. 統計分析 (コンソール出力)
# ==========================================
print("\n" + "="*50)
print(f"=== 分析結果: {os.path.basename(FILE_PROPOSED)} ===")
print("="*50)

# グループ分け（既存 vs 提案）
common = merged.dropna(subset=['time_existing', 'time_proposed'])
new_users = merged[merged['time_existing'].isna() & merged['time_proposed'].notna()]

# --- 1. 共通ユーザの分析 ---
print(f"1. [既存 vs 提案] 共通して情報を知ったデータ数: {len(common)} 件")
if not common.empty:
    avg_common_no = common['time_existing'].mean()
    avg_common_with = common['time_proposed'].mean()
    diff = avg_common_no - avg_common_with
    
    print(f"   - 既存手法(UAVなし) 平均時刻: {avg_common_no:.2f} s")
    print(f"   - 提案手法(UAVあり) 平均時刻: {avg_common_with:.2f} s")
    if diff > 0:
        print(f"   -> 判定: 提案手法の方が【{diff:.2f} 秒早い】")
    else:
        print(f"   -> 判定: 提案手法の方が {abs(diff):.2f} 秒遅い")

print("-" * 30)

# --- 2. 避難所巡回データの簡易統計 ---
print(f"2. [避難所巡回] データ数: {len(t_pt)} 件")
print(f"   - 平均受信時刻: {t_pt['time_patrol'].mean():.2f} s")
print("-" * 30)

print("\n=== 結論 ===")
total_captured_no = len(t_ex)
total_captured_pt = len(t_pt)
total_captured_with = len(t_pr)
print(f"総受信数(延べ): {total_captured_no}(既存) vs {total_captured_pt}(巡回) vs {total_captured_with}(提案)")

# ==========================================
# 4. グラフ作成: 散布図 (既存 vs 提案)
# ==========================================
# ※散布図は2軸なので「既存 vs 提案」のままにします
count_proposed_faster = (common['time_existing'] > common['time_proposed']).sum()
count_same = (common['time_existing'] == common['time_proposed']).sum()
count_existing_faster = (common['time_existing'] < common['time_proposed']).sum()

print("\nグラフを作成中 (散布図)...")
plt.figure(figsize=(8, 8))
max_val = max(common['time_existing'].max(), common['time_proposed'].max()) * 1.05
plt.fill_between([0, max_val], 0, [0, max_val], color='mistyrose', alpha=0.5)

plt.scatter(common['time_existing'], common['time_proposed'], 
            alpha=0.5, s=25, c='purple', edgecolors='white', linewidth=0.3, zorder=2, label='共通受信データ')
plt.plot([0, max_val], [0, max_val], 'k--', linewidth=1.5, zorder=3, label='等速線 (y=x)')

plt.title('情報初回受信時刻の比較 (既存 vs 提案)', fontsize=16)
plt.xlabel('既存手法の受信時刻 [秒]', fontsize=14)
plt.ylabel('提案手法の受信時刻 [秒]', fontsize=14)
plt.xlim(0, max_val)
plt.ylim(0, max_val)
plt.grid(True, linestyle=':', alpha=0.6)
plt.gca().set_aspect('equal', adjustable='box')

stats_text = (f"内訳 (総数 {len(common)}):\n"
              f"--------------------\n"
              f"提案が早い: {count_proposed_faster} 件\n"
              f"同じ　　　: {count_same} 件\n"
              f"既存が早い: {count_existing_faster} 件")
plt.text(max_val*0.05, max_val*0.95, stats_text, fontsize=12, verticalalignment='top',
         bbox=dict(boxstyle="round,pad=0.5", facecolor='white', alpha=0.9, edgecolor='gray'))

plt.text(max_val*0.6, max_val*0.1, '赤いエリア:\n提案手法の方が早い', 
         fontsize=12, color='red', bbox=dict(facecolor='white', alpha=0.8, edgecolor='red'))
plt.legend(fontsize=12, loc='upper right')

plt.tight_layout()
output_path_scatter = os.path.join(OUTPUT_DIR, 'Comparison_Scatter_Count.png')
plt.savefig(output_path_scatter, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_scatter}")

# ==========================================
# 5. グラフ作成: CDF (3手法比較)
# ==========================================
print("グラフを作成中 (CDF - 3手法比較)...")
plt.figure(figsize=(10, 6))

times_ex = t_ex['time_existing'].sort_values()
times_pt = t_pt['time_patrol'].sort_values() # 巡回
times_pr = t_pr['time_proposed'].sort_values()

# 分母の計算: 全3ファイルに含まれるユニークな(userID, blocknode)の総数
# これにより、どの手法でも到達できなかったケース以外を母数とします
all_pairs = pd.concat([
    t_ex[['userID', 'blocknode']],
    t_pt[['userID', 'blocknode']],
    t_pr[['userID', 'blocknode']]
]).drop_duplicates()
total_pairs = len(all_pairs)

print(f"CDF分母(全ユニークペア数): {total_pairs}")

# CDF作成
y_ex = np.arange(1, len(times_ex) + 1) / total_pairs
y_pt = np.arange(1, len(times_pt) + 1) / total_pairs
y_pr = np.arange(1, len(times_pr) + 1) / total_pairs

# プロット
plt.plot(times_ex, y_ex, label='w/o UAV', color='blue', linestyle='--', linewidth=2.5)
plt.plot(times_pt, y_pt, label='UAV避難所巡回', color='green', linestyle='-.', linewidth=2.5) # 追加
plt.plot(times_pr, y_pr, label='UAV通行止め重点飛行', color='red', linestyle='-', linewidth=2.5)

# plt.title('情報の累積受信率 (3手法比較)', fontsize=16)
plt.xlabel('シミュレーション時間 [秒]', fontsize=24)
plt.ylabel('情報所持率', fontsize=24)
plt.legend(fontsize=20, loc='lower right')
plt.grid(True, linestyle=':', alpha=0.6)

plt.tick_params(labelsize=18)

# 軸調整
max_x = max(times_ex.max() if not times_ex.empty else 0, 
            times_pt.max() if not times_pt.empty else 0,
            times_pr.max() if not times_pr.empty else 0) * 1.05
max_y = max(y_ex.max() if not times_ex.empty else 0,
            y_pt.max() if not times_pt.empty else 0,
            y_pr.max() if not times_pr.empty else 0) * 1.05

plt.xlim(0, max_x)
plt.ylim(0, max_y)

plt.tight_layout()
output_path_cdf = os.path.join(OUTPUT_DIR, 'Comparison_CDF_JP_3Methods.png')
plt.savefig(output_path_cdf, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_cdf}")

print("\n全処理が完了しました。")