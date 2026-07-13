import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os

# ====== 1. Load the CSV/TXT file ======
file_path = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/300/AoI_Info/ALL_AoI_Info_1.txt"
try:
    df = pd.read_csv(file_path)
except FileNotFoundError:
    print(f"Error: '{file_path}' not found. Please check the file path.")
    exit()

# ====== 2. Determine common ranges for all data ======
# When determining the color range, exclude 'unreceived' data (where time == AoI)
aoi_filtered = df[df['time'] != df['AoI']]['AoI']

x_min_global, x_max_global = df['x'].min(), df['x'].max()
y_min_global, y_max_global = df['y'].min(), df['y'].max()
aoi_min_global, aoi_max_global = aoi_filtered.min(), aoi_filtered.max()


# ====== 3. Get unique shelter IDs ======
unique_shelters = df['exitnode'].unique()
GRID_SIZE = (150, 150)

# ====== 4. Loop through each shelter to create a heatmap ======
for shelter in unique_shelters:
    print(f"--- Processing started for shelter: {shelter} ---")

    df_shelter = df[df['exitnode'] == shelter].copy()

    if df_shelter.empty:
        print(f"No data for shelter {shelter}. Skipping.")
        continue

    # --- Grid binning process ---
    x_bins = np.linspace(x_min_global, x_max_global, GRID_SIZE[1] + 1)
    y_bins = np.linspace(y_min_global, y_max_global, GRID_SIZE[0] + 1)

    df_shelter['x_bin_index'] = np.digitize(df_shelter['x'], bins=x_bins) - 1
    df_shelter['y_bin_index'] = np.digitize(df_shelter['y'], bins=y_bins) - 1

    df_shelter['x_bin_index'] = np.clip(df_shelter['x_bin_index'], 0, GRID_SIZE[1] - 1)
    df_shelter['y_bin_index'] = np.clip(df_shelter['y_bin_index'], 0, GRID_SIZE[0] - 1)

    # --- Create animation frames ---
    t_vals = sorted(df_shelter["time"].unique())
    AoI_frames = []

    for t in t_vals:
        sub_df_t = df_shelter[df_shelter["time"] == t]
        Z = np.full(GRID_SIZE, np.nan)

        # 1. Calculate the mean for normal data (time != AoI) and place it on the grid
        normal_df = sub_df_t[sub_df_t['time'] != sub_df_t['AoI']]
        if not normal_df.empty:
            grouped_normal = normal_df.groupby(['y_bin_index', 'x_bin_index'], as_index=False)['AoI'].mean()
            y_indices_norm = grouped_normal['y_bin_index'].values.astype(int)
            x_indices_norm = grouped_normal['x_bin_index'].values.astype(int)
            aoi_values_norm = grouped_normal['AoI'].values
            Z[y_indices_norm, x_indices_norm] = aoi_values_norm

        # 2. Identify cells containing unreceived data (time == AoI)
        unreceived_df = sub_df_t[sub_df_t['time'] == sub_df_t['AoI']]
        if not unreceived_df.empty:
            unreceived_indices = unreceived_df[['y_bin_index', 'x_bin_index']].drop_duplicates()
            y_indices_unrec = unreceived_indices['y_bin_index'].values.astype(int)
            x_indices_unrec = unreceived_indices['x_bin_index'].values.astype(int)

            # 3. Set a value greater than vmax for these cells to color them black
            Z[y_indices_unrec, x_indices_unrec] = aoi_max_global + 1

        AoI_frames.append(Z)

    # --- Plotting setup ---
    fig, ax = plt.subplots(figsize=(10, 8))

    cmap = plt.get_cmap('viridis_r')
    cmap.set_over('black')
    # ¥¥¥¥¥yCHANGEz¥¥¥¥¥
    # Set the color for NaN (no data) cells to white
    cmap.set_bad('white')
    # £££££yEND CHANGEz£££££

    cax = ax.imshow(AoI_frames[0], cmap=cmap, origin="lower",
                    extent=[x_min_global, x_max_global, y_min_global, y_max_global],
                    vmin=aoi_min_global, vmax=aoi_max_global)

    fig.colorbar(cax, label="AoI", extend='max')
    ax.set_xlabel("X Coordinate")
    ax.set_ylabel("Y Coordinate")

    # --- Animation update function ---
    def update(frame):
        cax.set_array(AoI_frames[frame])
        ax.set_title(f"AoI Heatmap - Shelter: {shelter} (Time={t_vals[frame]:.2f})")
        return [cax]

    # --- Animation generation and saving ---
    ani = animation.FuncAnimation(fig, update, frames=len(AoI_frames), interval=200, blit=True)

    sanitized_shelter_name = shelter.replace('/', '_')
    output_filename = f"aoi_animation_{sanitized_shelter_name}.mp4"

    print(f"Saving animation: {output_filename}")
    ani.save(output_filename, writer="ffmpeg", dpi=150)

    plt.close(fig)

    print(f"--- Processing complete for shelter: {shelter} ---")

print("\nAll processing has been completed.")