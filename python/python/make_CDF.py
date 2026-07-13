import matplotlib.pyplot as plt
import csv
import os

plt.rcParams["font.family"] = "IPAGothic"

# ======= 入力ファイル =======
filenames = [
    "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/Time/non-ferry.txt",
    "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/Time/ferry-around-exit.txt",
    "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/Time/ferry.txt"
]

# ======= 凡例ラベル =======
labels = [
    "フェリーノードなし",
    "フェリーノード避難所周回",
    "フェリーノード通行止め巡回"
]

# ======= チェック =======
if len(filenames) != len(labels):
    raise ValueError("filenames と labels の数が一致していません")

plt.figure(figsize=(9, 6))

# ======= CDF 作成 =======
for filename, label in zip(filenames, labels):
    times = []

    with open(filename, "r", encoding="utf-8") as f:
        reader = csv.reader(f)
        header = next(reader)

        for row in reader:
            if len(row) < 2:
                continue
            times.append(float(row[1]))

    sorted_times = sorted(times)
    n = len(sorted_times)
    
    # 0–1 → 0–100（％へ変換）
    cdf_percent = [(i + 1) / n * 100 for i in range(n)]

    plt.step(sorted_times, cdf_percent, where="post", label=label, linewidth=2)

# ======= 軸ラベルなど（文字大きめ）=======
plt.xlabel("避難時間(s)", fontsize=14, labelpad=8)
plt.ylabel(" 避難率(%)", fontsize=14, labelpad=8)
# plt.title("CDF Comparison of Evacuation Completion Times", fontsize=16)
plt.grid(True)

plt.legend(fontsize=12)

# ======= 軸範囲を0スタートに修正 =======
plt.xlim(left=0)   # x軸 0 から
plt.ylim(0, 100)   # y軸 0〜100%

# 左余白調整
plt.subplots_adjust(left=0.12)

output_path = "CDF_Comparison.png"  # 保存先（変更可）
plt.savefig(output_path, dpi=300, bbox_inches="tight")

plt.show()

print(f"PNG保存完了: {output_path}")

