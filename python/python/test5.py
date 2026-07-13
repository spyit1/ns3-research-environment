import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# ==========================================
# 1. �t�@�C���ݒ�i�����t�H���_�ɒu���j
# ==========================================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

FILE_EXISTING = os.path.join(BASE_DIR, "Recv_data_100.txt")
FILE_PROPOSED = os.path.join(BASE_DIR, "testLog_100.txt")

OUTPUT_DIR = os.path.join(BASE_DIR, "graph_output")
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

# ==========================================
# 2. �f�[�^�ǂݍ���
# ==========================================
print("Loading data...")

cols = ['time', 'userID', 'blocknode', 'AoI']

# �w�b�_�����{��؂蕶�������Ή�
df_ex = pd.read_csv(
    FILE_EXISTING,
    header=None,
    names=cols,
    sep=r"\s+|,",
    engine="python"
)

df_pr = pd.read_csv(
    FILE_PROPOSED,
    header=None,
    names=cols,
    sep=r"\s+|,",
    engine="python"
)

# �����񐮌`
df_ex['blocknode'] = df_ex['blocknode'].astype(str).str.strip()
df_pr['blocknode'] = df_pr['blocknode'].astype(str).str.strip()

# ==========================================
# 3. �L���f�[�^���o
# ==========================================
def filter_valid_info(df):
    return df[df['time'] > (df['AoI'] + 1.0)].copy()

valid_ex = filter_valid_info(df_ex)
valid_pr = filter_valid_info(df_pr)

def get_first_times(df):
    return df.groupby(['userID', 'blocknode'])['time'].min().reset_index()

t_ex = get_first_times(valid_ex).rename(columns={'time': 'time_existing'})
t_pr = get_first_times(valid_pr).rename(columns={'time': 'time_proposed'})

merged = pd.merge(t_ex, t_pr, on=['userID', 'blocknode'], how='outer')

# ���ʍő�l�i���O���t�p�j
max_val = max(
    t_ex['time_existing'].max() if len(t_ex) > 0 else 0,
    t_pr['time_proposed'].max() if len(t_pr) > 0 else 0
)

# ==========================================
# 4. Scatter Plot
# ==========================================
print("Generating Scatter Plot...")

plt.figure(figsize=(8, 8))

common = merged.dropna(subset=['time_existing', 'time_proposed'])

plt.scatter(
    common['time_existing'],
    common['time_proposed'],
    alpha=0.6,
    s=20,
    c='purple'
)

plt.plot([0, max_val], [0, max_val], 'k--', linewidth=1.5)

plt.title('First Reception Time Comparison')
plt.xlabel('Reception Time (Existing) [s]')
plt.ylabel('Reception Time (Proposed) [s]')
plt.grid(True)
plt.axis('equal')

plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, 'Comparison_Scatter.png'), dpi=300)
plt.close()

# ==========================================
# 5. CDF Plot
# ==========================================
print("Generating CDF Graph...")

plt.figure(figsize=(10, 6))

times_ex = t_ex['time_existing'].sort_values()
times_pr = t_pr['time_proposed'].sort_values()

total_pairs = len(merged) if len(merged) > 0 else 1

y_ex = np.arange(1, len(times_ex) + 1) / total_pairs
y_pr = np.arange(1, len(times_pr) + 1) / total_pairs

plt.plot(times_ex, y_ex, linestyle='--', linewidth=2)
plt.plot(times_pr, y_pr, linestyle='-', linewidth=2)

plt.title('Cumulative Information Diffusion Rate')
plt.xlabel('Simulation Time [s]')
plt.ylabel('Ratio of Informed User-Block Pairs')
plt.grid(True)

plt.xlim(0, max_val)
plt.ylim(0, 1.05)

plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, 'Comparison_CDF.png'), dpi=300)
plt.close()

print("Done. Graphs saved in 'graph_output' folder.")