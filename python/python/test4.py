import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# ==========================================
# 1. 設定項目
# ==========================================
# ファイルパス (環境に合わせて変更してください)
FILE_EXISTING = "/home/miki/ns-allinone-3.35/ns-3.35/obayashiIOFiles/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/10/AoI_Info/ALL_Blocked_AoI_Info_1.txt"  # _1 (Existing)
FILE_PROPOSED = "/home/miki/ns-allinone-3.35/ns-3.35/obayashiIOFiles/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/10/AoI_Info/ALL_Blocked_AoI_Info_30.txt" # _3 (Proposed)

# 出力先フォルダ
OUTPUT_DIR = 'graph_output'
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

# ==========================================
# 2. データ読み込みと前処理
# ==========================================
print("Loading data...")
cols = ['time', 'userID', 'blocknode', 'AoI']

try:
    df_ex = pd.read_csv(FILE_EXISTING, usecols=cols)
    df_pr = pd.read_csv(FILE_PROPOSED, usecols=cols)
except FileNotFoundError as e:
    print(f"Error: {e}")
    exit()

# 文字列の空白削除
df_ex['blocknode'] = df_ex['blocknode'].astype(str).str.strip()
df_pr['blocknode'] = df_pr['blocknode'].astype(str).str.strip()

# 【重要】有効な情報受信のみを抽出 (time > AoI + 1.0)
# これにより、まだ情報を持っていない初期状態を除外します
def filter_valid_info(df):
    return df[df['time'] > (df['AoI'] + 1.0)].copy()

valid_ex = filter_valid_info(df_ex)
valid_pr = filter_valid_info(df_pr)

# 「初めて情報を知った時刻」を取得
def get_first_times(df):
    return df.groupby(['userID', 'blocknode'])['time'].min().reset_index()

t_ex = get_first_times(valid_ex).rename(columns={'time': 'time_existing'})
t_pr = get_first_times(valid_pr).rename(columns={'time': 'time_proposed'})

# データを結合 (Outer Joinで全員網羅)
merged = pd.merge(t_ex, t_pr, on=['userID', 'blocknode'], how='outer')

# ==========================================
# 3. グラフ1: 散布図 (Scatter Plot) - 速さの比較
# ==========================================
print("Generating Scatter Plot...")
plt.figure(figsize=(8, 8))

# 両方の手法で受信できたユーザのみ抽出
common = merged.dropna(subset=['time_existing', 'time_proposed'])

# プロット
plt.scatter(common['time_existing'], common['time_proposed'], 
            alpha=0.6, s=20, c='purple', label='Common Users', edgecolors='white', linewidth=0.5)

# 対角線 (y=x) を引く
max_val = max(common['time_existing'].max(), common['time_proposed'].max())
plt.plot([0, max_val], [0, max_val], 'k--', linewidth=1.5, label='Equal Speed (y=x)')

# 装飾
plt.title('First Reception Time Comparison', fontsize=16)
plt.xlabel('Reception Time (Existing Method) [s]', fontsize=14)
plt.ylabel('Reception Time (Proposed Method) [s]', fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, linestyle=':', alpha=0.6)
plt.axis('equal') # 縦横比を1:1にして比較しやすくする

# 「対角線より下が提案手法の勝ち」という注釈
plt.text(max_val*0.6, max_val*0.1, 'Lower Right Area:\nProposed is Faster', 
         fontsize=12, color='red', bbox=dict(facecolor='white', alpha=0.8))

plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, 'Comparison_Scatter.png'), dpi=300)
plt.close()


# ==========================================
# 4. グラフ2: CDF (累積分布関数) - カバレッジの比較
# ==========================================
print("Generating CDF Graph...")
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
plt.plot(times_ex, y_ex, label='Existing Method (No UAV)', color='blue', linestyle='--', linewidth=2.5)
plt.plot(times_pr, y_pr, label='Proposed Method (With UAV)', color='red', linestyle='-', linewidth=2.5)

# 装飾
plt.title('Cumulative Information Diffusion Rate', fontsize=16)
plt.xlabel('Simulation Time [s]', fontsize=14)
plt.ylabel('Ratio of Informed User-Block Pairs', fontsize=14)
plt.legend(fontsize=12, loc='lower right')
plt.grid(True, linestyle=':', alpha=0.6)
plt.xlim(0, max_val)
plt.ylim(0, max(y_pr.max(), y_ex.max()) * 1.05)

plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, 'Comparison_CDF.png'), dpi=300)
plt.close()

print(f"Done. Graphs saved in '{OUTPUT_DIR}' folder.")