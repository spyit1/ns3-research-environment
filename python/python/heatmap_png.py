import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

# ====== 1. t@CpXÆ^[QbgÔÌÝè ======
aoi_file_path = "/home/miki/ns-allinone-3.35/ns-3.35/obayashiIOFiles/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/10/AoI_Info/ALL_AoI_Info_100.txt"
shelter_locations_path = "/home/miki/ns-allinone-3.35/ns-3.35/obayashiIOFiles/Log/obayashi/ON/QuickHello_OFF/EraseBlock_OFF/10/Info/Exit/Exit_Nodes_xy100.txt"

# --- ¥¥¥ ÏX_ ¥¥¥ ---
#画像出力したいTimeを指定
TARGET_TIME = 125.0
# --- £££ ÏX_ £££ ---

# ====== 2. f[^ÌÇÝÝ ======
try:
    df = pd.read_csv(aoi_file_path)
    shelter_locations_df = pd.read_csv(shelter_locations_path)
except FileNotFoundError as e:
    print(f"Error: A file was not found. Please check the path. Details: {e}")
    exit()

# ====== 3. ¤ÊÌÍÍÆJ[}bvðÝè ======
aoi_filtered = df[df['time'] != df['AoI']]['AoI']
x_min_global, x_max_global = df['x'].min(), df['x'].max()
y_min_global, y_max_global = df['y'].min(), df['y'].max()
aoi_min_global, aoi_max_global = aoi_filtered.min(), aoi_filtered.max()
print(aoi_filtered.max())

cmap = plt.get_cmap('viridis_r')
norm = plt.Normalize(vmin=aoi_min_global, vmax=aoi_max_global)

# ====== 4. eðï²ÆÌæð¶¬ ======
unique_shelters = df['exitnode'].unique()
for shelter in unique_shelters:
    # print(f"--- Processing started for shelter: {shelter} at Time={TARGET_TIME} ---")

    df_shelter = df[df['exitnode'] == shelter].copy()
    if df_shelter.empty:
        print(f"No data for shelter {shelter}. Skipping.")
        continue
    
    # --- ¥¥¥ ÏX_ ¥¥¥ ---
    # wèµ½ÔÌf[^ÌÝðo
    sub_df_t = df_shelter[df_shelter["time"] == TARGET_TIME]

    if sub_df_t.empty:
        print(f"No data for shelter {shelter} at Time={TARGET_TIME}. Skipping.")
        continue
    # --- £££ ÏX_ £££ ---

    shelter_info = shelter_locations_df[shelter_locations_df['exitnodeid'] == shelter]

    # --- `æ (Aj[VÅÍÈ­¼Úvbg) ---
    fig, ax = plt.subplots(figsize=(10, 8))
    mappable = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
    fig.colorbar(mappable, ax=ax, label="AoI")
    
    normal_df = sub_df_t[sub_df_t['time'] != sub_df_t['AoI']]
    unreceived_df = sub_df_t[sub_df_t['time'] == sub_df_t['AoI']]

    if not normal_df.empty:
        ax.scatter(normal_df['x'], normal_df['y'], c=normal_df['AoI'], cmap=cmap, norm=norm, s=20, alpha=0.8)
    if not unreceived_df.empty:
        ax.scatter(unreceived_df['x'], unreceived_df['y'], c='black', s=20, alpha=0.8)
    
    if not shelter_info.empty:
        ax.scatter(shelter_info['x'].iloc[0], shelter_info['y'].iloc[0], 
                   marker='x', color='red', s=200, linewidths=3, zorder=10)
    
    # ax.set_title(f"AoI Scatter Plot - Shelter: {shelter} (Time={TARGET_TIME:.2f})") # ^CgÉÔð\¦
    ax.set_xlabel("X(m)")
    ax.set_ylabel("Y(m)")
    ax.set_xlim(x_min_global, x_max_global)
    ax.set_ylim(y_min_global, y_max_global)
    ax.set_aspect('equal', adjustable='box')
    ax.grid(True, linestyle='--', alpha=0.6)

    # --- ¥¥¥ ÏX_ ¥¥¥ ---
    # Aj[VÛ¶ÌãíèÉAæt@CÆµÄÛ¶
    sanitized_shelter_name = shelter.replace('/', '_')
    output_filename = f"aoi_scatter_{sanitized_shelter_name}_time{int(TARGET_TIME)}.png"
    print(f"Saving image: {output_filename}")
    plt.savefig(output_filename, dpi=150)
    plt.close(fig) # ððú·é½ßÉ}ðÂ¶é
    # --- £££ ÏX_ £££ ---
    
    print(f"--- Processing complete for shelter: {shelter} ---")


# ====== 5. Sðïðµ½æð¶¬ ======
print(f"\n--- Processing started for ALL shelters combined at Time={TARGET_TIME} ---")

# --- ¥¥¥ ÏX_ ¥¥¥ ---
# wèµ½ÔÌf[^ÌÝðo
df_combined_t = df[df["time"] == TARGET_TIME]

if df_combined_t.empty:
    print(f"No data for combined plot at Time={TARGET_TIME}. Exiting.")
else:
    # --- `æ ---
    fig_all, ax_all = plt.subplots(figsize=(10, 8))
    mappable_all = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
    cbar = fig_all.colorbar(mappable_all, ax=ax_all)
    cbar.set_label("User's Average AoI")

    # 1. f[^ª
    normal_df = df_combined_t[df_combined_t['time'] != df_combined_t['AoI']]
    unreceived_df = df_combined_t[df_combined_t['time'] == df_combined_t['AoI']]

    # 2. ½ÏAoIðvZµÄvbg
    if not normal_df.empty:
        user_avg_aoi = normal_df.groupby(['userID', 'x', 'y']).agg(mean_aoi=('AoI', 'mean')).reset_index()
        ax_all.scatter(user_avg_aoi['x'], user_avg_aoi['y'], c=user_avg_aoi['mean_aoi'], cmap=cmap, norm=norm, s=20, alpha=0.8)

    # 3. ¢óMm[hðvbg
    if not unreceived_df.empty:
        if not normal_df.empty:
            received_user_ids = set(normal_df['userID'])
            true_unreceived_df = unreceived_df[~unreceived_df['userID'].isin(received_user_ids)]
        else:
            true_unreceived_df = unreceived_df
        
        if not true_unreceived_df.empty:
            plot_unreceived = true_unreceived_df.drop_duplicates(subset=['userID'])
            ax_all.scatter(plot_unreceived['x'], plot_unreceived['y'], c='black', s=20, alpha=0.8)

    # ðïðvbg
    ax_all.scatter(shelter_locations_df['x'], shelter_locations_df['y'],
                   marker='x', color='red', s=200, linewidths=3, zorder=10)

    # ax_all.set_title(f"AoI Scatter Plot - All Shelters (Time={TARGET_TIME:.2f})") # ^CgÉÔð\¦
    ax_all.set_xlabel("X(m)")
    ax_all.set_ylabel("Y(m)")
    ax_all.set_xlim(x_min_global, x_max_global)
    ax_all.set_ylim(y_min_global, y_max_global)
    ax_all.set_aspect('equal', adjustable='box')
    ax_all.grid(True, linestyle='--', alpha=0.6)

    # æÆµÄÛ¶
    output_filename_all = f"aoi_scatter_ALL_shelters_time{int(TARGET_TIME)}.png"
    print(f"Saving combined image: {output_filename_all}")
    plt.savefig(output_filename_all, dpi=150)
    plt.close(fig_all)
    print("--- Processing complete for the combined image ---")
# --- £££ ÏX_ £££ ---

print("\nAll processing has been completed.")