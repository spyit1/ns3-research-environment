import pandas as pd
import matplotlib.pyplot as plt

def plot_aoi_comparison_by_shelter(file1_path, file2_path):
    """
    2つのログファイルを読み込み、各避難所ごとにAoIの推移を比較するグラフを作成して保存します。
    """
    # データの読み込み
    try:
        df1 = pd.read_csv(file1_path)
        df2 = pd.read_csv(file2_path)
    except Exception as e:
        print(f"ファイルの読み込みに失敗しました: {e}")
        return

    # 両方のファイルに含まれる避難所（exitnode）のリストを取得
    shelters = set(df1['exitnode'].unique()) | set(df2['exitnode'].unique())
    print(f"対象となる避難所: {shelters}")

    for shelter in shelters:
        # 各避難所のデータを抽出
        data1 = df1[df1['exitnode'] == shelter]
        data2 = df2[df2['exitnode'] == shelter]
        
        # データがどちらにも存在しない場合はスキップ
        if data1.empty and data2.empty:
            continue

        # 時間ごとにAoIの平均値を計算
        # (個々のユーザーのAoIを平均化して全体の傾向を見ます)
        ts1 = data1.groupby('time')['AoI'].mean()
        ts2 = data2.groupby('time')['AoI'].mean()
        
        # グラフの作成
        plt.figure(figsize=(10, 6))
        
        if not ts1.empty:
            plt.plot(ts1.index, ts1.values, label='Quick Hello', alpha=0.8)
        if not ts2.empty:
            plt.plot(ts2.index, ts2.values, label='Hello = 60', alpha=0.8, linestyle='--')
        
        plt.title(f'AoI Comparison for Shelter: {shelter}')
        plt.xlabel('Time')
        plt.ylabel('Average AoI')
        plt.legend()
        plt.grid(True)
        
        # ファイル名に使えない文字（/など）を置換して保存
        safe_name = str(shelter).replace('/', '_')
        output_file = f'AoI_Comparison_{safe_name}.png'
        plt.savefig(output_file)
        plt.close() # メモリ解放のためClose
        
        print(f'{output_file} を保存しました。')

# --- 実行例 ---
# 実際のファイルパスに書き換えて実行してください
usernum = "699"
run = "7"
file_a = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_AoI_Info_'+run+'.txt'
file_b = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_AoI_Info_'+run+'.txt'
plot_aoi_comparison_by_shelter(file_a, file_b)