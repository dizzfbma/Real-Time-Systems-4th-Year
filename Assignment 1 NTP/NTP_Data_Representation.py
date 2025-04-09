import re
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

def parse_ntp_output_file(filepath):

    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    data_rows = []
    current_timestamp = None
    in_ntp_data = False

    for line in lines:

        ts_match = re.search(r"\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\]\s+NTP Query Result", line)
        if ts_match:

            current_timestamp = datetime.strptime(ts_match.group(1), "%Y-%m-%d %H:%M:%S")
            in_ntp_data = True
            continue



        if in_ntp_data:
            # Skip lines that are just "====" or "remote"
            if line.strip().startswith("====") or line.strip().startswith("remote") or not line.strip():
                continue

            parts = line.split()
            if len(parts) >= 10:
                # e.g. "meg.magnet.ie   217.53.21.92     2 u    1   64    1    9.171   +0.892   0.000"
                server_col = parts[0].lstrip("+-*")
                try:
                    delay_val = float(parts[7])
                    offset_str = parts[8].replace("+", "")
                    offset_val = float(offset_str)
                    jitter_val = float(parts[9])
                except ValueError:
                    # If parse fails, skip
                    continue

                if current_timestamp:
                    row = {
                        "Timestamp": current_timestamp,
                        "Server": server_col,
                        "Delay": delay_val,
                        "Offset": offset_val,
                        "Jitter": jitter_val
                    }
                    data_rows.append(row)

    return data_rows

def analyze_ntp_data(df):
    """Compute per-server statistics (min, max, mean, std) for Delay and Jitter."""
    stats = df.groupby("Server")[["Delay", "Jitter"]].agg(["min", "max", "mean", "std"])
    return stats

def plot_delay_jitter(df):
    """Plot Delay vs. Time and Jitter vs. Time, x-axis = UTC time."""
    df = df.sort_values("Timestamp")

    # Plot Delay
    plt.figure(figsize=(10,5))
    for server in df["Server"].unique():
        subset = df[df["Server"] == server]
        plt.plot(subset["Timestamp"], subset["Delay"], marker="o", label=server)
    plt.xlabel("UTC Time")
    plt.ylabel("Delay (ms)")
    plt.title("NTP Delay Over Time")
    plt.xticks(rotation=45)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    # Plot Jitter
    plt.figure(figsize=(10,5))
    for server in df["Server"].unique():
        subset = df[df["Server"] == server]
        plt.plot(subset["Timestamp"], subset["Jitter"], marker="o", label=server)
    plt.xlabel("UTC Time")
    plt.ylabel("Jitter (ms)")
    plt.title("NTP Jitter Over Time")
    plt.xticks(rotation=45)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    filepath = "ntp_output.txt"
    rows = parse_ntp_output_file(filepath)
    df = pd.DataFrame(rows)
    if df.empty:
        print("No valid NTP data found. Check file format or script logic.")
    else:
        print("\nParsed NTP Data:\n", df)

        # Stats
        stats_df = analyze_ntp_data(df)
        print("\nStats (min, max, mean, std) for Delay & Jitter:\n", stats_df)

        # Plots
        plot_delay_jitter(df)


