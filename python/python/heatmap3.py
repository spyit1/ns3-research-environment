import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os

# ====== 1. Load the data files ======
aoi_file_path = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/999/AoI_Info/ALL_AoI_Info_1.txt"
# ??????NEW??????
shelter_locations_path = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/999/Info/Exit/Exit_Nodes_xy1.txt" 
# ??????END NEW??????

try:
    df = pd.read_csv(aoi_file_path)
    # ??????NEW??????
    shelter_locations_df = pd.read_csv(shelter_locations_path)
    # ??????END NEW??????
except FileNotFoundError as e:
    print(f"Error: A file was not found. Please check the path. Details: {e}")
    exit()

# ====== 2. Determine common ranges for all data ======
aoi_filtered = df[df['time'] != df['AoI']]['AoI']
x_min_global, x_max_global = df['x'].min(), df['x'].max()
y_min_global, y_max_global = df['y'].min(), df['y'].max()
aoi_min_global, aoi_max_global = aoi_filtered.min(), aoi_filtered.max()

# ====== 3. Get unique shelter IDs ======
unique_shelters = df['exitnode'].unique()

# ====== 4. Loop through each shelter to create a heatmap ======
for shelter in unique_shelters:
    print(f"--- Processing started for shelter: {shelter} ---")

    df_shelter = df[df['exitnode'] == shelter].copy()

    if df_shelter.empty:
        print(f"No data for shelter {shelter}. Skipping.")
        continue
        
    # ??????NEW: Get shelter coordinates??????
    shelter_info = shelter_locations_df[shelter_locations_df['exitnodeid'] == shelter]
    if not shelter_info.empty:
        shelter_x = shelter_info['x'].iloc[0]
        shelter_y = shelter_info['y'].iloc[0]
        has_shelter_location = True
    else:
        has_shelter_location = False
        print(f"Warning: Location for shelter {shelter} not found in {shelter_locations_path}.")
    # ??????END NEW??????

    t_vals = sorted(df_shelter["time"].unique())

    # --- Plotting setup ---
    fig, ax = plt.subplots(figsize=(10, 8))
    ax.set_xlim(x_min_global, x_max_global)
    ax.set_ylim(y_min_global, y_max_global)
    ax.set_aspect('equal', adjustable='box')

    cmap = plt.get_cmap('viridis_r')
    norm = plt.Normalize(vmin=aoi_min_global, vmax=aoi_max_global)
    mappable = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
    fig.colorbar(mappable, ax=ax, label="AoI")

    # --- Animation update function ---
    def update(frame):
        ax.clear()
        
        current_time = t_vals[frame]
        sub_df_t = df_shelter[df_shelter["time"] == current_time]

        normal_df = sub_df_t[sub_df_t['time'] != sub_df_t['AoI']]
        unreceived_df = sub_df_t[sub_df_t['time'] == sub_df_t['AoI']]

        if not normal_df.empty:
            ax.scatter(normal_df['x'], normal_df['y'], c=normal_df['AoI'], cmap=cmap, norm=norm, s=20, alpha=0.8)
        if not unreceived_df.empty:
            ax.scatter(unreceived_df['x'], unreceived_df['y'], c='black', s=20, alpha=0.8)
        
        # ??????NEW: Plot shelter marker??????
        if has_shelter_location:
            ax.scatter(shelter_x, shelter_y, 
                       marker='x',        # Set marker shape to 'x'
                       color='red',       # Set color to red
                       s=200,             # Set marker size
                       linewidths=3,      # Set line width of the marker
                       zorder=10)         # Ensure it's drawn on top of other points
        # ??????END NEW??????
        
        ax.set_title(f"AoI Scatter Plot - Shelter: {shelter} (Time={current_time:.2f})")
        ax.set_xlabel("X Coordinate")
        ax.set_ylabel("Y Coordinate")
        ax.set_xlim(x_min_global, x_max_global)
        ax.set_ylim(y_min_global, y_max_global)
        ax.set_aspect('equal', adjustable='box')
        ax.grid(True, linestyle='--', alpha=0.6)

    # --- Animation generation and saving ---
    ani = animation.FuncAnimation(fig, update, frames=len(t_vals), interval=200, blit=False)

    sanitized_shelter_name = shelter.replace('/', '_')
    output_filename = f"aoi_scatter_{sanitized_shelter_name}.mp4"

    print(f"Saving animation: {output_filename}")
    ani.save(output_filename, writer="ffmpeg", dpi=150)

    plt.close(fig)

    print(f"--- Processing complete for shelter: {shelter} ---")

print("\nAll processing has been completed.")