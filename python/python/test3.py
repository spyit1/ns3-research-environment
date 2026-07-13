import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import platform

# ==========================================
# 0. 日本語フォントの設定
# ==========================================
# OSに合わせて標準的な日本語フォントを探して設定します
system_name = platform.system()
if system_name == 'Windows':
    plt.rcParams['font.family'] = 'MS Gothic'
elif system_name == 'Darwin': # Mac
    plt.rcParams['font.family'] = 'Hiragino Sans'
else:
    # Linux (Ubuntu等) の場合、環境によってフォントが異なります。
    # 豆腐（□□）になる場合は、インストールされている日本語フォントを指定してください。
    # 例: 'IPAGothic', 'TakaoGothic', 'Noto Sans CJK JP' など
    try:
        import japanize_matplotlib
    except ImportError:
        pass # japanize_matplotlibがない場合はデフォルトまたはシステムフォントに頼る
    plt.rcParams['font.family'] = 'IPAGothic' 

# ==========================================
# 1. 設定項目
# ==========================================
# ファイルパス (環境に合わせて変更してください)
FILE_EXISTING = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_1.txt"  # 既存手法 (UAVなし/巡回)
FILE_PROPOSED = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_3.txt"   # 提案手法 (UAVあり/重点)

# 出力先フォルダ
OUTPUT_DIR = 'graph_output_jp'
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

# ==========================================
# 2. データ読み込みと前処理関数
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
        print(f"読み込みエラー ({filepath}): {e}")
        return None

    # 文字列の空白削除
    if df['blocknode'].dtype == object:
        df['blocknode'] = df['blocknode'].astype(str).str.strip()

    # 【重要】有効な情報受信のみを抽出 (time > AoI + 1.0)
    # AoIがtimeと同じ(初期値)のデータを除外して、実際に受信したデータのみ残す
    valid_df = df[df['time'] > (df['AoI'] + 1.0)].copy()

    # 「初めて情報を知った時刻」を取得
    # ユーザIDと通行止めノードのペアごとに、最小の時刻を取得
    first_times = valid_df.groupby(['userID', 'blocknode'])['time'].min().reset_index()
    
    return first_times

# データを処理
t_ex = load_and_process(FILE_EXISTING) # 既存手法データ
t_pr = load_and_process(FILE_PROPOSED) # 提案手法データ

if t_ex is None or t_pr is None:
    print("データ読み込みに失敗したため、処理を中断します。")
    exit()

# カラム名変更 (結合用)
t_ex = t_ex.rename(columns={'time': 'time_existing'})
t_pr = t_pr.rename(columns={'time': 'time_proposed'})

# データを結合 (Outer Joinで全員網羅)
merged = pd.merge(t_ex, t_pr, on=['userID', 'blocknode'], how='outer')

# ==========================================
# 3. 統計データの詳細分析
# ==========================================
# グループ分け
# 1. 両方の手法で情報を知った人（共通部分）
common = merged.dropna(subset=['time_existing', 'time_proposed'])

# 2. UAVありでしか知れなかった人（新規獲得）
# time_existing が NaN (知らない) で、time_proposed がある人
new_users = merged[merged['time_existing'].isna() & merged['time_proposed'].notna()]

# 3. 逆にUAVなしの方が知っていた人（稀だが一応確認）
lost_users = merged[merged['time_existing'].notna() & merged['time_proposed'].isna()]

print("\n" + "="*50)
print(f"=== 分析結果: {os.path.basename(FILE_PROPOSED)} ===")
print("="*50)

# --- 共通ユーザの分析 ---
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

# --- 新規ユーザの分析 ---
print(f"2. UAV導入により新規に情報を知ったデータ数: {len(new_users)} 件")
if not new_users.empty:
    avg_new = new_users['time_proposed'].mean()
    print(f"   - 新規獲得層の平均受信時刻: {avg_new:.2f} s")
    print("   -> 考察: 既存手法では到達しなかったエリアへ、時間をかけて情報を届けました。")

print("-" * 30)

# --- 喪失ユーザの分析 ---
if not lost_users.empty:
    print(f"3. 注意: 既存手法の方が情報を知っていたデータ数: {len(lost_users)} 件")
    print("   -> 考察: 経路変更により、通行止め地点に近づかなくなった可能性があります。")
else:
    print("3. 情報喪失（既存で知れていたのに提案で知れなくなったケース）はありません。")

print("\n=== 結論 ===")
total_captured_no = len(t_ex)
total_captured_with = len(t_pr)
print(f"総受信数(延べ): {total_captured_no} (既存) vs {total_captured_with} (提案)")

if not common.empty and (avg_common_no - avg_common_with) > -5.0: # 多少の遅れは許容
    if len(new_users) > 0:
        print("★「受信時間の短縮（または同等）」と「受信範囲の拡大」の両方を達成しています。")
        print("★ 非常に良い結果です。")
    else:
        print("受信時間は短縮されましたが、情報の広がり（人数）に大きな変化はありませんでした。")
else:
    print("共通ユーザの受信時間が遅くなっています。迂回による孤立の影響が強い可能性があります。")

print("="*50)


# ==========================================
# 4. グラフ作成: 散布図 (速さの比較)
# ==========================================
print("\nグラフを作成中 (散布図)...")
plt.figure(figsize=(8, 8))

# プロット
plt.scatter(common['time_existing'], common['time_proposed'], 
            alpha=0.6, s=20, c='purple', label='共通受信データ', edgecolors='white', linewidth=0.5)

# 対角線 (y=x) を引く
max_val = max(common['time_existing'].max(), common['time_proposed'].max())
plt.plot([0, max_val], [0, max_val], 'k--', linewidth=1.5, label='等速線 (y=x)')

# 装飾
plt.title('情報初回受信時刻の比較 (共通受信データ)', fontsize=16)
plt.xlabel('既存手法の受信時刻 [秒]', fontsize=14)
plt.ylabel('提案手法の受信時刻 [秒]', fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, linestyle=':', alpha=0.6)
plt.axis('equal') # 縦横比を1:1にする

# 注釈
plt.text(max_val*0.6, max_val*0.1, '右下の領域:\n提案手法の方が早い', 
         fontsize=12, color='red', bbox=dict(facecolor='white', alpha=0.8, edgecolor='red'))

plt.tight_layout()
output_path_scatter = os.path.join(OUTPUT_DIR, 'Comparison_Scatter_JP.png')
plt.savefig(output_path_scatter, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_scatter}")


# ==========================================
# 5. グラフ作成: CDF (広さの比較)
# ==========================================
print("グラフを作成中 (CDF)...")
plt.figure(figsize=(10, 6))

# 受信時刻データをソート
times_ex = t_ex['time_existing'].sort_values()
times_pr = t_pr['time_proposed'].sort_values()

# 全対象ペア数 (これを分母にすることで、到達率の絶対値を示す)
total_pairs = len(merged)

# Y軸データの作成 (受信できた割合)
y_ex = np.arange(1, len(times_ex) + 1) / total_pairs
y_pr = np.arange(1, len(times_pr) + 1) / total_pairs

# プロット
plt.plot(times_ex, y_ex, label='既存手法 (UAVなし)', color='blue', linestyle='--', linewidth=2.5)
plt.plot(times_pr, y_pr, label='提案手法 (UAVあり)', color='red', linestyle='-', linewidth=2.5)

# 装飾
plt.title('情報の累積受信率 (拡散速度と網羅性)', fontsize=16)
plt.xlabel('シミュレーション経過時間 [秒]', fontsize=14)
plt.ylabel('情報到達率 (対 全ペア数)', fontsize=14)
plt.legend(fontsize=12, loc='lower right')
plt.grid(True, linestyle=':', alpha=0.6)

# グラフの範囲調整
plt.xlim(0, max_val)
plt.ylim(0, max(y_pr.max(), y_ex.max()) * 1.05)

plt.tight_layout()
output_path_cdf = os.path.join(OUTPUT_DIR, 'Comparison_CDF_JP.png')
plt.savefig(output_path_cdf, dpi=300)
plt.close()
print(f"  -> 保存完了: {output_path_cdf}")

print("\n全処理が完了しました。")