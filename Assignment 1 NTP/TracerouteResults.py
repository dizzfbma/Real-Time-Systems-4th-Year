import re
import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

def parse_traceroute_file(filepath):
    
    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    traceroutes = []
    current_target = None
    hops = []

    for line in lines:
        # 1) Identify the start of a new traceroute
        target_match = re.search(r"Tracing route to (\S+)", line)
        if target_match:
            # If we already have a target and some hops, store them
            if current_target and hops:
                traceroutes.append((current_target, hops))
            current_target = target_match.group(1)
            hops = []
            continue

        # 2) Ignore lines with "Request timed out."
        if "Request timed out" in line:
            continue

        # 3) Attempt to parse a hop line, e.g.:


        hop_match = re.search(
            r"^\s*(\d+)\s+(\d+)\s*ms\s+(\d+)\s*ms\s+(\d+)\s*ms\s+(.*)$",
            line
        )
        if hop_match:
            hop_number = int(hop_match.group(1))
            time_1 = int(hop_match.group(2))
            time_2 = int(hop_match.group(3))
            time_3 = int(hop_match.group(4))
            hop_address = hop_match.group(5).strip()

            # Average the three times
            avg_time = (time_1 + time_2 + time_3) / 3.0
            hops.append((hop_number, avg_time, hop_address))


    if current_target and hops:
        traceroutes.append((current_target, hops))

    return traceroutes


def build_dataframe(traceroutes):

    rows = []
    for (target, hop_list) in traceroutes:
        for hop_number, avg_latency, hop_addr in hop_list:
            rows.append([target, hop_number, avg_latency, hop_addr])
    df = pd.DataFrame(rows, columns=["Target", "Hop", "Avg Latency (ms)", "Hop Address"])
    return df


def visualize_traceroutes(traceroutes):

    G = nx.DiGraph()

    for target, hops in traceroutes:
        previous_node = "Source"
        for hop_number, avg_latency, hop_address in hops:
            hop_node = f"Hop {hop_number} ({hop_address})"
            # Edge with weight=avg_latency
            G.add_edge(previous_node, hop_node, weight=avg_latency)
            previous_node = hop_node
        # Final edge from last hop to "Destination: <target>"
        G.add_edge(previous_node, f"Destination: {target}")

    # Draw the graph
    plt.figure(figsize=(12, 6))
    pos = nx.spring_layout(G, k=0.3)
    nx.draw(
        G, pos, with_labels=True, node_color="lightblue", edge_color="gray",
        node_size=3000, font_size=8
    )
    # Add edge labels with latency
    edge_labels = {
        (u, v): f"{G[u][v]['weight']:.1f} ms"
        for (u, v) in G.edges if "weight" in G[u][v]
    }
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=7)
    plt.title("Traceroute Graph Visualization")
    plt.show()


def plot_hop_vs_latency(df):

    if df.empty:
        print("No data to plot.")
        return

    plt.figure(figsize=(10, 5))
    for target in df["Target"].unique():
        subset = df[df["Target"] == target]
        plt.plot(subset["Hop"], subset["Avg Latency (ms)"], marker="o", label=target)

    plt.xlabel("Hop Number")
    plt.ylabel("Average Latency (ms)")
    plt.title("Hop Count vs. Latency for Each Traceroute Target")
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    # 1) Parse the file
    filepath = "traceroute_results.txt"  # Adjust if needed
    traceroutes = parse_traceroute_file(filepath)

    # 2) Convert to DataFrame
    df = build_dataframe(traceroutes)
    print("\nTraceroute DataFrame:\n", df)

    # 3) If we have data, visualize
    if not df.empty:
        visualize_traceroutes(traceroutes)
        plot_hop_vs_latency(df)
    else:
        print("No valid traceroute data found. Check file format or lines with 'ms' times.")


