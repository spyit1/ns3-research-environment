import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os

# Set font for Japanese characters (to prevent garbled text)
# Please specify a font installed on your system
# (e.g., 'MS Gothic', 'Yu Gothic', 'Hiragino Sans')
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.sans-serif'] = ['Hiragino Maru Gothic Pro', 'Yu Gothic', 'Meiryo', 'Takao', 'IPAexGothic', 'IPAPGothic', 'VL PGothic', 'Noto Sans CJK JP']

# ====== 1. Load the CSV file ======
try:
    df = pd.read_csv("/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/300/AoI_Info/ALL_AoI_Info_1.txt")
except FileNotFoundError:
    print("Error: 'shelter_aoi.csv' not found. Please check the filename and path.")
    exit()

# ====== 2. Determine common ranges for all data ======
# Determine global min/max values to unify axes and color bars across all heatmaps
x_min_global, x_max_global = df['x'].min(), df['x'].max()
y_min_global, y_max_global = df['y'].min(), df['y'].max()
aoi_min_global, aoi_max_global = df['AoI'].min(), df['AoI'].max()

# ====== 3. Get unique shelter IDs ======
unique_shelters = df['exitnode'].unique()

GRID_SIZE = (100, 100)

# ====== 4. Loop through each shelter to create a heatmap ======
for shelter in unique_shelters:
    print(f"--- Processing started for shelter: {shelter} ---")
    
    # Filter data for the current shelter only
    df_shelter = df[df['exitnode'] == shelter].copy()
    
    if df_shelter.empty:
        print(f"No data found for shelter {shelter}. Skipping.")
        continue

    # --- Grid processing ---
    x_bins = np.linspace(x_min_global, x_max_global, GRID_SIZE[1] + 1)
    y_bins = np.linspace(y_min_global, y_max_global, GRID_SIZE[0] + 1)
    
    df_shelter['x_bin_index'] = np.digitize(df_shelter['x'], bins=x_bins) - 1
    df_shelter['y_bin_index'] = np.digitize(df_shelter['y'], bins=y_bins) - 1

    df_shelter['x_bin_index'] = np.clip(df_shelter['x_bin_index'], 0, GRID_SIZE[1] - 1)
    df_shelter['y_bin_index'] = np.clip(df_shelter['y_bin_index'], 0, GRID_SIZE[0] - 1)

    # --- Animation frame creation ---
    t_vals = sorted(df_shelter["time"].unique())
    AoI_frames = []
    
    for t in t_vals:
        sub_df = df_shelter[df_shelter["time"] == t]
        Z = np.full(GRID_SIZE, np.nan)
        grouped = sub_df.groupby(['y_bin_index', 'x_bin_index'], as_index=False)['AoI'].mean()
        
        y_indices = grouped['y_bin_index'].values.astype(int)
        x_indices = grouped['x_bin_index'].values.astype(int)
        aoi_values = grouped['AoI'].values
        
        Z[y_indices, x_indices] = aoi_values
        AoI_frames.append(Z)

    # --- Plot setup ---
    fig, ax = plt.subplots(figsize=(10, 8))
    
    cax = ax.imshow(AoI_frames[0], cmap="coolwarm", origin="lower",
                    extent=[x_min_global, x_max_global, y_min_global, y_max_global],
                    vmin=aoi_min_global, vmax=aoi_max_global)
    
    fig.colorbar(cax, label="AoI")
    ax.set_xlabel("X Coordinate")
    ax.set_ylabel("Y Coordinate")
    
    # --- Animation update function ---
    def update(frame):
        cax.set_array(AoI_frames[frame])
        # Set the title with shelter name and time
        ax.set_title(f"AoI Heatmap - Shelter: {shelter} (Time={t_vals[frame]:.2f})")
        return [cax]

    # --- Animation generation and saving ---
    ani = animation.FuncAnimation(fig, update, frames=len(AoI_frames), interval=200, blit=True)
    
    # Sanitize the shelter name for the filename by replacing '/' with '_'
    sanitized_shelter_name = shelter.replace('/', '_')
    output_filename = f"aoi_animation_{sanitized_shelter_name}.mp4"
    
    print(f"Saving animation: {output_filename}")
    ani.save(output_filename, writer="ffmpeg", dpi=150)
    
    # Close the figure to free up memory for the next loop
    plt.close(fig)
    
    print(f"--- Processing complete for shelter: {shelter} ---")

print("\nAll processing complete.")