import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os

# ====== 0. 出力ディレクトリ指定 ======
OUTPUT_DIRECTORY = "/home/obayashi/ns-allinone-3.35/ns-3.35/heatmap/exit"
os.makedirs(OUTPUT_DIRECTORY, exist_ok=True)
print(f"動画は {OUTPUT_DIRECTORY} に保存されます．")

# ====== 1. Load the data files ======
aoi_file_path = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/AoI_Info/ALL_AoI_Info_3.txt"
shelter_locations_path = "/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/499/Info/Exit/Exit_Nodes_xy3.txt"

try:
    df = pd.read_csv(aoi_file_path)
    shelter_locations_df = pd.read_csv(shelter_locations_path)
except FileNotFoundError as e:
    print(f"Error: file not found． {e}")
    exit()

# ====== 2. Determine common ranges ======
aoi_filtered = df[df['time'] != df['AoI']]['AoI']
x_min_global, x_max_global = df['x'].min(), df['x'].max()
y_min_global, y_max_global = df['y'].min(), df['y'].max()
aoi_min_global, aoi_max_global = aoi_filtered.min(), aoi_filtered.max()

# ====== y 座標反転関数 ======
def invert_y(y):
    return y_max_global + y_min_global - y

# ====== 3. Colormap ======
cmap = plt.get_cmap('viridis_r')
norm = plt.Normalize(vmin=aoi_min_global, vmax=aoi_max_global)

# ====== 4. Individual shelter videos ======
unique_shelters = df['exitnode'].unique()

for shelter in unique_shelters:
    print(f"--- Processing shelter: {shelter} ---")

    df_shelter = df[df['exitnode'] == shelter].copy()
    if df_shelter.empty:
        continue

    shelter_info = shelter_locations_df[shelter_locations_df['exitnodeid'] == shelter]
    t_vals = sorted(df_shelter["time"].unique())

    fig, ax = plt.subplots(figsize=(10, 8))
    mappable = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
    fig.colorbar(mappable, ax=ax, label="AoI")

    def update_individual(frame):
        ax.clear()
        current_time = t_vals[frame]
        sub_df_t = df_shelter[df_shelter["time"] == current_time]

        normal_df = sub_df_t[sub_df_t['time'] != sub_df_t['AoI']]
        unreceived_df = sub_df_t[sub_df_t['time'] == sub_df_t['AoI']]

        if not normal_df.empty:
            ax.scatter(
                normal_df['x'],
                invert_y(normal_df['y']),
                c=normal_df['AoI'],
                cmap=cmap, norm=norm, s=20, alpha=0.8
            )

        if not unreceived_df.empty:
            ax.scatter(
                unreceived_df['x'],
                invert_y(unreceived_df['y']),
                c='black', s=20, alpha=0.8
            )

        if not shelter_info.empty:
            ax.scatter(
                shelter_info['x'].iloc[0],
                invert_y(shelter_info['y'].iloc[0]),
                marker='x', color='red', s=200, linewidths=3, zorder=10
            )

        ax.set_title(f"AoI Scatter Plot - Shelter: {shelter} (Time={current_time:.2f})")
        ax.set_xlabel("X (m)")
        ax.set_ylabel("Y (m)")
        ax.set_xlim(x_min_global, x_max_global)
        ax.set_ylim(y_min_global, y_max_global)
        ax.set_aspect('equal', adjustable='box')
        ax.grid(True, linestyle='--', alpha=0.6)

    ani = animation.FuncAnimation(fig, update_individual, frames=len(t_vals), interval=200)

    output_filename = f"aoi_scatter_{shelter.replace('/', '_')}.mp4"
    ani.save(os.path.join(OUTPUT_DIRECTORY, output_filename), writer="ffmpeg", dpi=150)
    plt.close(fig)

    print(f"--- Finished shelter: {shelter} ---")

# ====== 5. Combined video ======
print("\n--- Processing ALL shelters ---")

t_vals_all = sorted(df["time"].unique())
fig_all, ax_all = plt.subplots(figsize=(10, 8))
mappable_all = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
fig_all.colorbar(mappable_all, ax=ax_all, label="User's Average AoI")

def update_combined(frame):
    ax_all.clear()
    current_time = t_vals_all[frame]
    sub_df_t = df[df["time"] == current_time]

    normal_df = sub_df_t[sub_df_t['time'] != sub_df_t['AoI']]
    unreceived_df = sub_df_t[sub_df_t['time'] == sub_df_t['AoI']]

    if not normal_df.empty:
        user_avg_aoi = normal_df.groupby(['userID', 'x', 'y']).agg(
            mean_aoi=('AoI', 'mean')
        ).reset_index()

        ax_all.scatter(
            user_avg_aoi['x'],
            invert_y(user_avg_aoi['y']),
            c=user_avg_aoi['mean_aoi'],
            cmap=cmap, norm=norm, s=20, alpha=0.8
        )

    if not unreceived_df.empty:
        received_user_ids = set(normal_df['userID']) if not normal_df.empty else set()
        true_unreceived_df = unreceived_df[~unreceived_df['userID'].isin(received_user_ids)]
        plot_unreceived = true_unreceived_df.drop_duplicates(subset=['userID'])

        if not plot_unreceived.empty:
            ax_all.scatter(
                plot_unreceived['x'],
                invert_y(plot_unreceived['y']),
                c='black', s=20, alpha=0.8
            )

    ax_all.scatter(
        shelter_locations_df['x'],
        invert_y(shelter_locations_df['y']),
        marker='x', color='red', s=200, linewidths=3, zorder=10
    )

    ax_all.set_title(f"AoI Scatter Plot - All Shelters (Time={current_time:.2f})")
    ax_all.set_xlabel("X (m)")
    ax_all.set_ylabel("Y (m)")
    ax_all.set_xlim(x_min_global, x_max_global)
    ax_all.set_ylim(y_min_global, y_max_global)
    ax_all.set_aspect('equal', adjustable='box')
    ax_all.grid(True, linestyle='--', alpha=0.6)

ani_all = animation.FuncAnimation(fig_all, update_combined, frames=len(t_vals_all), interval=200)

output_all = os.path.join(OUTPUT_DIRECTORY, "aoi_scatter_ALL_shelters.mp4")
ani_all.save(output_all, writer="ffmpeg", dpi=150)
plt.close(fig_all)

print("\nAll processing completed．")
