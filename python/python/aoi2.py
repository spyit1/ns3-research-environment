import pandas as pd
import matplotlib.pyplot as plt

def plot_aoi_comparison_informed_only(file1_path, file2_path):
    """
    「情報を持っているユーザー（AoI < time）」のみを対象に、
    各避難所ごとのAoI平均値を比較するグラフを作成します。
    """
    # データの読み込み
    try:
        df1 = pd.read_csv(file1_path)
        df2 = pd.read_csv(file2_path)
    except Exception as e:
        print(f"ファイルの読み込みに失敗しました: {e}")
        return

    # 両方のファイルに含まれる避難所のリストを取得
    shelters = set(df1['exitnode'].unique()) | set(df2['exitnode'].unique())
    print(f"対象となる避難所: {shelters}")

    for shelter in shelters:
        # その避難所のデータ、かつ「情報を持っている(AoI < time)」行だけを抽出
        # ※ time=0 の時は AoI=0 なので通常ここに含まれません（グラフは時刻1以降から描画される傾向になります）
        data1 = df1[(df1['exitnode'] == shelter) & (df1['AoI'] < df1['time'])]
        data2 = df2[(df2['exitnode'] == shelter) & (df2['AoI'] < df2['time'])]
        
        # データがなければスキップ
        if data1.empty and data2.empty:
            print(f"Skip: {shelter} (有効な情報を持つユーザーがいません)")
            continue

        # 時間ごとにAoIの平均値を計算
        # ここでの分母は「その時刻に情報を持っているユーザー数」になります
        ts1 = data1.groupby('time')['AoI'].mean()
        ts2 = data2.groupby('time')['AoI'].mean()
        
        # グラフの作成
        plt.figure(figsize=(10, 6))
        
        if not ts1.empty:
            plt.plot(ts1.index, ts1.values, label='Quick Helo', alpha=0.8)
        if not ts2.empty:
            plt.plot(ts2.index, ts2.values, label='Hello = 60', alpha=0.8, linestyle='--')
        
        plt.title(f'Average AoI (Informed Users Only)\nShelter: {shelter}')
        plt.xlabel('Time')
        plt.ylabel('Average AoI')
        plt.legend()
        plt.grid(True)
        
        # ファイル名用にスラッシュを置換
        safe_name = str(shelter).replace('/', '_')
        output_file = f'AoI_Informed_{safe_name}.png'
        plt.savefig(output_file)
        plt.close()
        
        print(f'{output_file} を保存しました。')

# --- 実行例 ---
usernum = "699"
run = "7"
file_a = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_AoI_Info_'+run+'.txt'
file_b = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/'+usernum+'/AoI_Info/ALL_AoI_Info_'+run+'.txt'
plot_aoi_comparison_informed_only(file_a, file_b)