import pandas as pd
import numpy as np

# ====== ファイル設定 ======
# ---------------------------------------------------------
# ファイルパス設定 (ユーザーの環境に合わせて変更してください)
# _1: UAVなし (Existing), _3: UAVあり (Proposed)
# ---------------------------------------------------------
FILE_NO_UAV = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_1.txt"
FILE_WITH_UAV = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_Blocked_AoI_Info_3.txt"

print("Loading data...")
cols = ['time', 'userID', 'blocknode', 'AoI']
try:
    df_no = pd.read_csv(FILE_NO_UAV, usecols=cols)
    df_with = pd.read_csv(FILE_WITH_UAV, usecols=cols)
except FileNotFoundError:
    print("Error: File not found.")
    exit()

# 文字列の空白削除
df_no['blocknode'] = df_no['blocknode'].astype(str).str.strip()
df_with['blocknode'] = df_with['blocknode'].astype(str).str.strip()

# ====== 【修正点】有効な情報受信のみを抽出 ======
# 「AoI < Time」となっている行だけが、有効な情報を持っている
# 念のため 1秒以上の差があるものを抽出（浮動小数点誤差対策）
def filter_valid_info(df):
    return df[df['time'] > (df['AoI'] + 1.0)].copy()

valid_no = filter_valid_info(df_no)
valid_with = filter_valid_info(df_with)

print(f"有効データ数 (UAVなし): {len(valid_no)} / {len(df_no)}")
print(f"有効データ数 (UAVあり): {len(valid_with)} / {len(df_with)}")

# ====== 「初めて情報を知った時刻」を取得 ======
def get_first_times(df):
    # ユーザ・通行止め地点ごとの最小時刻を取得
    return df.groupby(['userID', 'blocknode'])['time'].min().reset_index()

t_no = get_first_times(valid_no).rename(columns={'time': 'time_no'})
t_with = get_first_times(valid_with).rename(columns={'time': 'time_with'})

# データを結合
merged = pd.merge(t_no, t_with, on=['userID', 'blocknode'], how='outer')

# ====== 分析 ======

# 1. 共通して情報を知った人（比較可能）
common = merged.dropna(subset=['time_no', 'time_with'])

# 2. UAVありでのみ知った人（新規獲得）
new_users = merged[merged['time_no'].isna() & merged['time_with'].notna()]

print("\n=== 【修正版】詳細分析結果 ===")

print(f"全ユーザ×全通行止めのペア数（最大 {500*8}=4000）のうち...")

# --- 共通受信者の比較 ---
print(f"\n1. 両方の手法で情報を知ったペア数: {len(common)} 組")
if not common.empty:
    avg_no = common['time_no'].mean()
    avg_with = common['time_with'].mean()
    diff = avg_no - avg_with
    
    print(f"   - UAVなし 平均受信時刻: {avg_no:.2f} s")
    print(f"   - UAVあり 平均受信時刻: {avg_with:.2f} s")
    print("-" * 20)
    if diff > 0:
        print(f"   ★ 結論: 同じ人同士で比べると、UAVありの方が平均【 {diff:.2f} 秒 】早く知りました！")
    else:
        print(f"   ▲ 結論: 同じ人同士だと {abs(diff):.2f} 秒遅いです（これは遠方のUAV経由受信が含まれるため等の理由が考えられます）。")

# --- 新規受信者の確認 ---
print(f"\n2. UAVなしでは知らず、UAVありで初めて知ったペア数: {len(new_users)} 組")
if not new_users.empty:
    avg_new = new_users['time_with'].mean()
    print(f"   - この人たちの平均受信時刻: {avg_new:.2f} s")
    print("   -> この人たちは、UAVがなければ「永遠に（または避難完了まで）知らなかった」人たちです。")

# --- 合計ユニーク人数 ---
u_common = common['userID'].nunique()
u_new = new_users['userID'].nunique()
print("\n------------------------------")
print(f"情報を知ったユニーク人数（実人数）:")
print(f"   - 共通: {u_common} 人")
print(f"   - 新規: {u_new} 人")
print(f"   -> 合計 {u_common + u_new} 人がUAVありで情報を知りました。")