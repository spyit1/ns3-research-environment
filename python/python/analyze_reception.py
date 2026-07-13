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
    # フォントが見つからない場合のフォールバック（豆腐化防止）
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
# ファイルパス (環境に合わせて変更してください)
FILE_EXISTING = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/AoI_Info/ALL_Blocked_AoI_Info_1.txt"
FILE_PROPOSED = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/299/AoI_Info/ALL_Blocked_AoI_Info_3.txt"

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

t_ex = load_and_process(FILE_EXISTING)
t_pr = load_and_process(FILE_PROPOSED)

if t_ex is None or t_pr is None:
    print("データ不足のため終了します。")
    exit()

# カラム名変更と結合
t_ex = t_ex.rename(columns={'time': 'time_existing'})
t_pr = t_pr.rename(columns={'time': 'time_proposed'})
merged = pd.merge(t_ex, t_pr, on=['userID', 'blocknode'], how='outer')

# ==========================================
# 3. 統計分析 (コンソール出力)
# ==========================================
print("\n" + "="*50)
print(f"=== 分析結果: {os.path.basename(FILE_PROPOSED)} ===")
print("="*50)

# グループ分け
common = merged.dropna(subset=['time_existing', 'time_proposed'])
new_users = merged[merged['time_existing'].isna() & merged['time_proposed'].notna()]
lost_users = merged[merged['time_existing'].notna() & merged['time_proposed'].isna()]

# --- 1. 共通ユーザの分析 ---
print(f"\n1. 共通して情報を知ったデータ数: {len(common)} 件")
avg_common_no = 0
avg_common_with = 0

if not common.empty:
    avg_common_no = common['time_existing'].mean()
    avg_common_with = common['time_proposed'].mean()
    diff = avg_common_no - avg_common_with
    
    print(f"   - 既存手法(UAVなし) 平均時刻: {avg_common_no:.2f} s")
    print(f"   - 提案手法(UAVあり) 平均時刻: {avg_common_with:.2f} s")
    
    if diff > 0:
        print(f"   -> 判定: 同じ人同士の比較では、提案手法の方が【{diff:.2f} 秒早い】です。")
    else:
        print(f"   -> 判定: 同じ人同士の比較では、提案手法の方が {abs(diff):.2f} 秒遅いです。")

print("-" * 30)

# --- 2. 新規ユーザの分析 ---
print(f"2. UAV導入により新規に情報を知ったデータ数: {len(new_users)} 件")
if not new_users.empty:
    avg_new = new_users['time_proposed'].mean()
    print(f"   - 新規獲得層の平均受信時刻: {avg_new:.2f} s")
    print("   -> 考察: 既存手法では到達しなかったエリアへ、時間をかけて情報を届けました。")

print("-" * 30)

# --- 3. 喪失ユーザの分析 ---
if not lost_users.empty:
    print(f"3. 注意: 既存手法の方が情報を知っていたデータ数: {len(lost_users)} 件")
else:
    print("3. 情報喪失（既存で知れていたのに提案で知れなくなったケース）はありません。")

print("\n=== 結論 ===")
total_captured_no = len(t_ex)
total_captured_with = len(t_pr)
print(f"総受信数(延べ): {total_captured_no} (既存) vs {total_captured_with} (提案)")

if not common.empty and (avg_common_no - avg_common_with) > -5.0:
    if len(new_users) > 0:
        print("★「受信時間の短縮」と「受信範囲の拡大」の両方を達成しています。")
        print("★ 非常に良い結果です。")
    else:
        print("受信時間は短縮されましたが、情報の広がり（人数）に変化はありませんでした。")
else:
    print("共通ユーザの受信時間が遅くなっています。")
print("="*50)

# ==========================================
# 4. グラフ作成: 散布図 (内訳カウント付き)
# ==========================================
# カウント計算
count_proposed_faster = (common['time_existing'] > common['time_proposed']).sum()
count_same = (common['time_existing'] == common['time_proposed']).sum()
count_existing_faster = (common['time_existing'] < common['time_proposed']).sum()

print("\nグラフを作成中 (散布図)...")
plt.figure(figsize=(8, 8))

# 描画範囲設定
max_val = max(common['time_existing'].max(), common['time_proposed'].max()) * 1.05

# 背景色 (提案有利エリア)
plt.fill_between([0, max_val], 0, [0, max_val], color='mistyrose', alpha=0.5)

# プロット (alpha=0.5 で重なりを可視化) 【★修正: labelを追加】
plt.scatter(common['time_existing'], common['time_proposed'], 
            alpha=0.5, s=25, c='purple', edgecolors='white', linewidth=0.3, zorder=2, label='共通受信データ')

# 対角線 【★修正: labelを追加】
plt.plot([0, max_val], [0, max_val], 'k--', linewidth=1.5, zorder=3, label='等速線 (y=x)')

# ラベル
plt.title('情報初回受信時刻の比較 (共通受信データ)', fontsize=16)
plt.xlabel('既存手法の受信時刻 [秒]', fontsize=14)
plt.ylabel('提案手法の受信時刻 [秒]', fontsize=14)
plt.xlim(0, max_val)
plt.ylim(0, max_val)
plt.grid(True, linestyle=':', alpha=0.6)
plt.gca().set_aspect('equal', adjustable='box')

# 件数表示ボックス
stats_text = (f"内訳 (総数 {len(common)}):\n"
              f"--------------------\n"
              f"提案が早い: {count_proposed_faster} 件\n"
              f"同じ　　　: {count_same} 件\n"
              f"既存が早い: {count_existing_faster} 件")
plt.text(max_val*0.05, max_val*0.95, stats_text, fontsize=12, verticalalignment='top',
         bbox=dict(boxstyle="round,pad=0.5", facecolor='white', alpha=0.9, edgecolor='gray'))

# エリア説明
plt.text(max_val*0.6, max_val*0.1, '赤いエリア:\n提案手法の方が早い', 
         fontsize=12, color='red', bbox=dict(facecolor='white', alpha=0.8, edgecolor='red'))

# 凡例 【★追加: 他のテキストと被らない右上配置】
plt.legend(fontsize=12, loc='upper right')

plt.tight_layout()
output_path_scatter = os.path.join(OUTPUT_DIR, 'Comparison_Scatter_Count.png')
plt.savefig(output_path_scatter, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_scatter}")

# ==========================================
# 5. グラフ作成: CDF (累積分布関数)
# ==========================================
print("グラフを作成中 (CDF)...")
plt.figure(figsize=(10, 6))

times_ex = t_ex['time_existing'].sort_values()
times_pr = t_pr['time_proposed'].sort_values()
total_pairs = len(merged) # 全ユニークペア数を分母にする

# CDF作成
y_ex = np.arange(1, len(times_ex) + 1) / total_pairs
y_pr = np.arange(1, len(times_pr) + 1) / total_pairs

plt.plot(times_ex, y_ex, label='w/o UAV', color='blue', linestyle='--', linewidth=2.5)
plt.plot(times_pr, y_pr, label='UAV通行止め重点飛行', color='red', linestyle='-', linewidth=2.5)

# plt.title('情報の累積受信率 (拡散速度と網羅性)', fontsize=16)
plt.xlabel('シミュレーション時間 [秒]', fontsize=14)
plt.ylabel('情報所持率', fontsize=14)
plt.legend(fontsize=12, loc='lower right')
plt.grid(True, linestyle=':', alpha=0.6)
plt.xlim(0, max_val)
plt.ylim(0, max(y_pr.max(), y_ex.max()) * 1.05)

plt.tight_layout()
output_path_cdf = os.path.join(OUTPUT_DIR, 'Comparison_CDF_JP.png')
plt.savefig(output_path_cdf, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_cdf}")

print("\n全処理が完了しました。")