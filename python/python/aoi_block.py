import pandas as pd
import matplotlib.pyplot as plt

def plot_aoi_comparison_by_blocknode(file1_path, file2_path):
    """
    2つのログファイルを読み込み、各 Block Node ごとにAoIの推移を比較するグラフを作成して保存します。
    （全ユーザー対象：未受信者はAoIが大きくなるため、平均値を押し上げます）
    """
    # データの読み込み
    try:
        df1 = pd.read_csv(file1_path)
        df2 = pd.read_csv(file2_path)
    except Exception as e:
        print(f"ファイルの読み込みに失敗しました: {e}")
        return

    # カラム名の確認（blocknodeがあるか）
    if 'blocknode' not in df1.columns:
        print("エラー: ログファイルに 'blocknode' カラムが見つかりません。")
        return

    # 両方のファイルに含まれる対象（blocknode）のリストを取得
    targets = set(df1['blocknode'].unique()) | set(df2['blocknode'].unique())
    print(f"対象となる Block Node: {targets}")

    for target in targets:
        # 各 Block Node のデータを抽出
        data1 = df1[df1['blocknode'] == target]
        data2 = df2[df2['blocknode'] == target]
        
        if data1.empty and data2.empty:
            continue

        # 時間ごとにAoIの平均値を計算
        ts1 = data1.groupby('time')['AoI'].mean()
        ts2 = data2.groupby('time')['AoI'].mean()
        
        # グラフの作成
        plt.figure(figsize=(10, 6))
        
        if not ts1.empty:
            plt.plot(ts1.index, ts1.values, label='File 1', alpha=0.8)
        if not ts2.empty:
            plt.plot(ts2.index, ts2.values, label='File 2', alpha=0.8, linestyle='--')
        
        plt.title(f'AoI Comparison for Block Node: {target}')
        plt.xlabel('Time')
        plt.ylabel('Average AoI')
        plt.legend()
        plt.grid(True)
        
        # ファイル名用にスラッシュなどを置換
        safe_name = str(target).replace('/', '_')
        output_file = f'AoI_Block_Comparison_{safe_name}.png'
        plt.savefig(output_file)
        plt.close()
        
        print(f'{output_file} を保存しました。')

# --- 実行例 ---
# 比較したい2つのファイルを指定してください
usernum = "699"
run = "7"
file_a = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_Blocked_AoI_Info_'+run+'.txt' 
file_b = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_Blocked_AoI_Info_'+run+'.txt' # 本来は別の条件のファイルを指定
plot_aoi_comparison_by_blocknode(file_a, file_b)