#!/usr/bin/env python3
"""
通行止め認知人数ログをグラフ化するスクリプト

想定CSV:
time,blocked_node_id,known_user_count,active_user_count,knowledge_rate

使用例:
  # 全通行止め箇所を個別にグラフ化
  python3 plot_blocked_node_knowledge.py BlockedNodeKnowledge_100.csv

  # 特定の通行止めIDだけグラフ化
  python3 plot_blocked_node_knowledge.py BlockedNodeKnowledge_100.csv --blocked-node-id 11027

  # 1000秒までに制限
  python3 plot_blocked_node_knowledge.py BlockedNodeKnowledge_100.csv --max-time 1000
"""

import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


REQUIRED_COLUMNS = {
    "time",
    "blocked_node_id",
    "known_user_count",
    "active_user_count",
    "knowledge_rate",
}


def safe_filename(value: object) -> str:
    """ファイル名に使用できない文字を置き換える。"""
    return re.sub(r"[^0-9A-Za-z_.-]+", "_", str(value))


def load_log(csv_path: Path) -> pd.DataFrame:
    """CSVを読み込み、必要な列とデータ型を確認する。"""
    if not csv_path.exists():
        raise FileNotFoundError(f"CSVファイルが見つかりません: {csv_path}")

    df = pd.read_csv(csv_path)

    missing = REQUIRED_COLUMNS - set(df.columns)
    if missing:
        raise ValueError(
            "CSVに必要な列がありません: "
            + ", ".join(sorted(missing))
            + "\n現在の列: "
            + ", ".join(df.columns)
        )

    numeric_columns = [
        "time",
        "known_user_count",
        "active_user_count",
        "knowledge_rate",
    ]

    for column in numeric_columns:
        df[column] = pd.to_numeric(df[column], errors="coerce")

    df = df.dropna(
        subset=[
            "time",
            "blocked_node_id",
            "known_user_count",
            "active_user_count",
            "knowledge_rate",
        ]
    )

    return df.sort_values(["blocked_node_id", "time"])


def plot_blocked_node(
    node_df: pd.DataFrame,
    blocked_node_id: object,
    output_dir: Path,
    show_graph: bool,
) -> Path:
    """1つの通行止め箇所について認知人数とActiveUser数を描画する。"""
    node_df = node_df.sort_values("time")

    fig, ax = plt.subplots(figsize=(12, 7))

    ax.plot(
        node_df["time"],
        node_df["known_user_count"],
        marker="o",
        markersize=3,
        linewidth=1.5,
        label="Users knowing blocked node",
    )

    ax.plot(
        node_df["time"],
        node_df["active_user_count"],
        linewidth=1.5,
        linestyle="--",
        label="Active users",
    )

    ax.set_title(f"Blocked node knowledge over time: {blocked_node_id}")
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Number of users")
    ax.grid(True, alpha=0.3)
    ax.legend()
    fig.tight_layout()

    output_path = output_dir / (
        f"blocked_node_{safe_filename(blocked_node_id)}_knowledge.png"
    )
    fig.savefig(output_path, dpi=300)

    if show_graph:
        plt.show()

    plt.close(fig)
    return output_path


def plot_knowledge_rate(
    node_df: pd.DataFrame,
    blocked_node_id: object,
    output_dir: Path,
    show_graph: bool,
) -> Path:
    """1つの通行止め箇所について認知率を描画する。"""
    node_df = node_df.sort_values("time")

    rate = node_df["knowledge_rate"].copy()

    # ログが0～1表記なら百分率へ変換する。
    if not rate.empty and rate.max() <= 1.0:
        rate = rate * 100.0

    fig, ax = plt.subplots(figsize=(12, 7))

    ax.plot(
        node_df["time"],
        rate,
        marker="o",
        markersize=3,
        linewidth=1.5,
    )

    ax.set_title(f"Blocked node knowledge rate over time: {blocked_node_id}")
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Knowledge rate [%]")
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()

    output_path = output_dir / (
        f"blocked_node_{safe_filename(blocked_node_id)}_knowledge_rate.png"
    )
    fig.savefig(output_path, dpi=300)

    if show_graph:
        plt.show()

    plt.close(fig)
    return output_path


def main() -> None:
    parser = argparse.ArgumentParser(
        description="通行止め認知人数ログを時系列グラフに変換します。"
    )

    parser.add_argument(
        "csv_path",
        type=Path,
        help="入力するBlockedNodeKnowledgeログのCSVファイル",
    )

    parser.add_argument(
        "--blocked-node-id",
        help="グラフ化する通行止めID。省略時は全IDを個別に出力",
    )

    parser.add_argument(
        "--max-time",
        type=float,
        help="この時刻までのデータだけを使用する（例: 1000）",
    )

    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("blocked_node_graphs"),
        help="グラフの保存先（既定: blocked_node_graphs）",
    )

    parser.add_argument(
        "--show",
        action="store_true",
        help="保存と同時にグラフを画面表示する",
    )

    args = parser.parse_args()

    df = load_log(args.csv_path)

    if args.max_time is not None:
        df = df[df["time"] <= args.max_time]

    if args.blocked_node_id is not None:
        # CSV側の型に左右されないよう文字列として比較する。
        df = df[
            df["blocked_node_id"].astype(str)
            == str(args.blocked_node_id)
        ]

        if df.empty:
            available_ids = sorted(
                load_log(args.csv_path)["blocked_node_id"]
                .astype(str)
                .unique()
            )
            raise ValueError(
                f"指定した通行止めIDが見つかりません: "
                f"{args.blocked_node_id}\n"
                f"利用可能なID: {', '.join(available_ids)}"
            )

    if df.empty:
        raise ValueError("条件に一致するデータがありません。")

    args.output_dir.mkdir(parents=True, exist_ok=True)

    output_files = []

    for blocked_node_id, node_df in df.groupby("blocked_node_id"):
        output_files.append(
            plot_blocked_node(
                node_df,
                blocked_node_id,
                args.output_dir,
                args.show,
            )
        )

        output_files.append(
            plot_knowledge_rate(
                node_df,
                blocked_node_id,
                args.output_dir,
                args.show,
            )
        )

    print(f"グラフを{len(output_files)}個保存しました。")
    for output_file in output_files:
        print(output_file)


if __name__ == "__main__":
    main()