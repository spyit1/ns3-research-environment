import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# ====== CSVt@CðÇÝÞ ======
df = pd.read_csv("/home/obayashi/ns-allinone-3.35/ns-3.35/Log/obayashi/ON/QuickHello_ON/EraseBlock_OFF/300/AoI_Info/AoI_Info_1.csv")

# ====== ObhÌðxðè` ======
GRID_SIZE = (100, 100) 

# ====== ÀWf[^ðObh»irªj======
x_min, x_max = df['x'].min(), df['x'].max()
y_min, y_max = df['y'].min(), df['y'].max()

x_bins = np.linspace(x_min, x_max, GRID_SIZE[1] + 1)
y_bins = np.linspace(y_min, y_max, GRID_SIZE[0] + 1)

df['x_bin_index'] = np.digitize(df['x'], bins=x_bins) - 1
df['y_bin_index'] = np.digitize(df['y'], bins=y_bins) - 1

df['x_bin_index'] = np.clip(df['x_bin_index'], 0, GRID_SIZE[1] - 1)
df['y_bin_index'] = np.clip(df['y_bin_index'], 0, GRID_SIZE[0] - 1)

# ====== eÌAoIObhf[^ðì¬ ======
t_vals = sorted(df["time"].unique())
AoI_frames = []

for t in t_vals:
    sub_df = df[df["time"] == t]
    
    Z = np.full(GRID_SIZE, np.nan) 
    
    grouped = sub_df.groupby(['y_bin_index', 'x_bin_index'], as_index=False)['AoI'].mean()
    
    y_indices = grouped['y_bin_index'].values.astype(int)
    x_indices = grouped['x_bin_index'].values.astype(int)
    aoi_values = grouped['AoI'].values
    
    Z[y_indices, x_indices] = aoi_values
    
    AoI_frames.append(Z)

# ====== Aj[VÌ`æ ======
fig, ax = plt.subplots()
aoi_min, aoi_max = df['AoI'].min(), df['AoI'].max()

# ¥¥¥¥¥y±±ªC³_z¥¥¥¥¥
# cmapð"inferno_r"ÉÏXµÄAAoIlªá¢ûª¾é­Èéæ¤ÉµÜ·
cax = ax.imshow(AoI_frames[0], cmap="coolwarm", origin="lower",
                extent=[x_min, x_max, y_min, y_max], vmin=aoi_min, vmax=aoi_max)
# £££££yC³±±ÜÅz£££££

fig.colorbar(cax, label="AoI")
ax.set_xlabel("X")
ax.set_ylabel("Y")

# ¥¥¥¥¥y±±ªÇÁ_z¥¥¥¥¥
# 避難所のx,y座標
marker_coords = [
    (1318.35, 1053.81),
    (1532.08, 2721.59),
    (2628.48, 1149.98)
]

def update(frame):
    cax.set_array(AoI_frames[frame])
    ax.set_title(f"AoI Heatmap (time={t_vals[frame]:.2f})")
    
    if hasattr(ax, 'marker_collection'):
        ax.marker_collection.remove()

    scatter_collection = ax.scatter(
        [p[0] for p in marker_coords],
        [p[1] for p in marker_coords],
        s=100,            # }[J[ÌTCY
        marker='X',       # }[J[Ì`ó ('o'ÍÛ, 's'Ílp, '^'Íãü«OpÈÇ)
        c='red',          # }[J[ÌF
        edgecolors='black', # }[J[ÌÌF
        linewidths=1,     # }[J[ÌÌ¾³
        zorder=5          # `æ (q[g}bvÌãÉ\¦)
    )
    ax.marker_collection = scatter_collection 

    return [cax, scatter_collection]

ani = animation.FuncAnimation(fig, update, frames=len(AoI_frames), interval=200, blit=True)

ani.save("aoi_animation.mp4", writer="ffmpeg", dpi=200)
plt.show()