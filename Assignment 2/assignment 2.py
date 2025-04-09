#!/usr/bin/env python3

import os
import pandas as pd
import matplotlib.pyplot as plt

def main():

    # Base path where scenario1..5 subfolders live
    scenario_base = os.path.join(".", "scenario")

    # List the scenario folders
    scenario_folders = [f"scenario{i}" for i in range(1, 6)]

    # Mapping of test_name -> column in CSV
    test_files = {
        "nanosleep": "Jitter_ns",
        "usleep": "Jitter_ns",
        "signal_latency": "Latency_ns",
        "timer": "Jitter_ns"
    }

    # Create an output folder for plots
    plots_dir = os.path.join(".", "plots")
    os.makedirs(plots_dir, exist_ok=True)

    # Create ONE figure per test_name
    for test_name, column_name in test_files.items():
        plt.figure(figsize=(8, 5))  # new figure for this test

        print(f"\n=== {test_name.capitalize()} Across All Scenarios ===")

        # Track if we actually plot anything
        scenario_count = 0

        for scenario_name in scenario_folders:
            scenario_path = os.path.join(scenario_base, scenario_name)
            csv_path = os.path.join(scenario_path, f"{test_name}.csv")

            if not os.path.isfile(csv_path):
                print(f"  [Warning] '{csv_path}' not found, skipping.")
                continue

            # Read the CSV
            try:
                df = pd.read_csv(csv_path)
            except Exception as e:
                print(f"  [Error] Could not read '{csv_path}': {e}")
                continue

            # Check required columns
            if "Iteration" not in df.columns or column_name not in df.columns:
                print(f"  [Error] Missing 'Iteration' or '{column_name}' in '{csv_path}', skipping.")
                continue

            # Extract data
            iteration = df["Iteration"]
            values = df[column_name]

            # Calculate stats
            count = values.count()
            minimum = values.min()
            maximum = values.max()
            mean = values.mean()
            std_dev = values.std()

            print(f"  {scenario_name}:")
            print(f"    Count: {count}")
            print(f"    Min:   {minimum:.2f}")
            print(f"    Max:   {maximum:.2f}")
            print(f"    Mean:  {mean:.2f}")
            print(f"    Std:   {std_dev:.2f}")

            # Plot on the current figure
            label_name = scenario_name
            plt.plot(iteration, values, label=label_name, marker='.', markersize=3, linestyle='-')

            scenario_count += 1

        # If plotted at least one scenario, finalize the figure
        if scenario_count > 0:
            plt.title(f"{test_name.capitalize()} Across Scenarios")
            plt.xlabel("Iteration")
            plt.ylabel(column_name)
            plt.legend()
            plt.grid(True)

            # Save figure
            out_filename = os.path.join(plots_dir, f"{test_name}_all_scenarios.png")
            plt.savefig(out_filename, dpi=150)
            plt.close()
            print(f"  Plot saved to '{out_filename}'")
        else:
            # No data was plotted
            plt.close()
            print(f"  [Info] No data found for {test_name} in any scenario.")

if __name__ == "__main__":
    main()
