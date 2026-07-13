import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

def load_block_coordinates(coord_file_path):
    """
    指定された座標リストファイルからBlock Nodeの座標を読み込む関数
    データ形式: blocknodeid, x, y
    """
    print(f"--- 座標定義ファイルの読み込み: {coord_file_path} ---")
    locations = {}
    
    try:
        # CSVとして読み込み
        df = pd.read_csv(coord_file_path)
        
        # 必要なカラムがあるか確認
        if not {'blocknodeid', 'x', 'y'}.issubset(df.columns):
            print("エラー: 座標ファイルのカラム形式が一致しません (blocknodeid, x, y が必要)")
            return locations

        for _, row in df.iterrows():
            # IDをキー、(x, y)のタプルを値として辞書に格納
            node_id = row['blocknodeid']
            locations[node_id] = (row['x'], row['y'])
            print(f"  [{node_id}] 確定座標: ({row['x']:.1f}, {row['y']:.1f})")
            
    except Exception as e:
        print(f"座標ファイル読み込みエラー: {e}")
    
    return locations

def plot_relevance_based_aoi_fixed(file_on, file_off, coord_file, radius=200):
    """
    固定座標を使用して、関連エリア（半径radius以内）のユーザーのみを評価したグラフを作成
    """
    # 1. 座標の読み込み (推定ではなくファイルから)
    block_locations = load_block_coordinates(coord_file)
    
    if not block_locations:
        print("有効な座標が読み込めませんでした。パスを確認してください。")
        return

    # 2. ログデータの読み込み
    try:
        print(f"--- ログ読み込み中... ---")
        df_on = pd.read_csv(file_on)
        df_off = pd.read_csv(file_off)
    except Exception as e:
        print(f"ログファイル読み込みエラー: {e}")
        return

    print(f"\n--- グラフ作成開始 (半径 {radius}m 以内のみ対象) ---")

    # 3. 各地点ごとに評価
    for block_name, (bx, by) in block_locations.items():
        # そのBlock Nodeに関連するデータのみ抽出
        # ※ログファイルの 'blocknode' カラムと 座標ファイルの 'blocknodeid' が一致している前提
        data_on = df_on[df_on['blocknode'] == block_name].copy()
        data_off = df_off[df_off['blocknode'] == block_name].copy()
        
        if data_on.empty and data_off.empty:
            print(f"Skip: {block_name} (ログ内に該当ノードのデータなし)")
            continue

        # --- 距離フィルタリング ---
        # ユーザーと確定座標(bx, by)との距離を計算
        data_on['dist'] = np.sqrt((data_on['x'] - bx)**2 + (data_on['y'] - by)**2)
        data_off['dist'] = np.sqrt((data_off['x'] - bx)**2 + (data_off['y'] - by)**2)
        
        # 「半径以内」かつ「情報を持っている(AoI < time)」ユーザーのみ抽出
        valid_on = data_on[(data_on['dist'] <= radius) & (data_on['AoI'] < data_on['time'])]
        valid_off = data_off[(data_off['dist'] <= radius) & (data_off['AoI'] < data_off['time'])]
        
        # 時系列で平均を計算
        ts_on = valid_on.groupby('time')['AoI'].mean()
        ts_off = valid_off.groupby('time')['AoI'].mean()
        
        # --- グラフ描画 ---
        if ts_on.empty and ts_off.empty:
            print(f"Skip: {block_name} (範囲内 {radius}m に有効なAoIデータなし)")
            continue

        plt.figure(figsize=(10, 6))
        
        if not ts_on.empty:
            plt.plot(ts_on.index, ts_on.values, label='Quick Hello ON', color='orange')
        if not ts_off.empty:
            plt.plot(ts_off.index, ts_off.values, label='Quick Hello OFF', color='blue', linestyle='--')
            
        plt.title(f'Relevant AoI Comparison (Radius < {radius}m)\nTarget: {block_name} (Fixed: {bx:.0f}, {by:.0f})')
        plt.xlabel('Time [s]')
        plt.ylabel('Average AoI [s]')
        plt.legend()
        plt.grid(True)
        
        # 保存 (ファイル名のスラッシュを置換)
        safe_name = block_name.replace('/', '_')
        output_file = f'Relevant_AoI_{safe_name}.png'
        plt.savefig(output_file)
        plt.close()
        print(f"-> Saved: {output_file}")

if __name__ == "__main__":
    # === 設定 ===
    usernum = "699" # ユーザー数ディレクトリ (必要に応じて変更してください)
    run = "7"       # 試行番号
    
    # 基本パス
    base_path = '/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON'
    
    # 1. 座標定義ファイル (Block_Nodes_xy7.txt)
    # ※ご提示のパス構造に合わせていますが、ファイル名が異なる場合は修正してください
    coord_file = base_path + '/QuickHello_ON/EraseBlock_OFF/'+usernum+'/Info/Block/Block_Nodes_xy'+run+'.txt'
    
    # 2. 比較するログファイル (ON / OFF)
    # ※usernumやrun変数がパスに含まれる場合は適宜調整してください
    file_on_path = f'{base_path}/QuickHello_ON/EraseBlock_OFF/{usernum}/AoI_Info/ALL_Blocked_AoI_Info_{run}.txt'
    file_off_path = f'{base_path}/QuickHello_OFF/EraseBlock_OFF/{usernum}/AoI_Info/ALL_Blocked_AoI_Info_{run}.txt'
    
    # ファイルの存在確認（念のため）
    if not os.path.exists(coord_file):
        print(f"警告: 座標ファイルが見つかりません -> {coord_file}")
    
    # === 実行 ===
    plot_relevance_based_aoi_fixed(file_on_path, file_off_path, coord_file, radius=100)