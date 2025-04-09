#!/usr/bin/env bash
#
# run_benchmarks.sh
#


# benchmark executable
BENCHMARK="./bm_combined"

# - NUM_PROCESSES_MEDIUM: how many pmg processes for "medium" CPU load
# - NUM_PROCESSES_HIGH: how many pmg processes for "high" CPU load
# - LIMIT_MEDIUM: upper limit passed to pmg for "medium" load
# - LIMIT_HIGH: upper limit passed to pmg for "high" load
NUM_PROCESSES_MEDIUM=2
NUM_PROCESSES_HIGH=4
LIMIT_MEDIUM=2000000    # 2 million
LIMIT_HIGH=5000000      # 5 million

# Helper function to launch multiple pmg processes in the background
run_pmg_load() {
  local num_procs="$1"
  local limit="$2"

  echo "Launching $num_procs pmg processes with limit=$limit ..."
  for i in $(seq 1 "$num_procs"); do
    # Each pmg instance runs in the background
    ./pmg "$limit" &
  done
}

# Helper function to kill background pmg processes
kill_pmg() {
  if pgrep -x pmg >/dev/null; then
    echo "Killing pmg processes..."
    pkill -x pmg
    wait 2>/dev/null
  fi
}

echo "=== Scenario 1: Low system load, no swapping ==="
kill_pmg
# Run the benchmark with no extra load
$BENCHMARK
mkdir -p scenario1
mv ./*.csv scenario1/

echo "=== Scenario 2: Medium CPU load, no swapping ==="
kill_pmg
run_pmg_load "$NUM_PROCESSES_MEDIUM" "$LIMIT_MEDIUM"
# Give pmg processes a moment to start
sleep 2
$BENCHMARK
kill_pmg
mkdir -p scenario2
mv ./*.csv scenario2/

echo "=== Scenario 3: High CPU load, no swapping ==="
kill_pmg
run_pmg_load "$NUM_PROCESSES_HIGH" "$LIMIT_MEDIUM"
sleep 2
$BENCHMARK
kill_pmg
mkdir -p scenario3
mv ./*.csv scenario3/

echo "=== Scenario 4: Medium CPU load, forced swapping (larger limit) ==="
kill_pmg
run_pmg_load "$NUM_PROCESSES_MEDIUM" "$LIMIT_HIGH"
sleep 2
$BENCHMARK
kill_pmg
mkdir -p scenario4
mv ./*.csv scenario4/

echo "=== Scenario 5: High CPU load, forced swapping (larger limit) ==="
kill_pmg
run_pmg_load "$NUM_PROCESSES_HIGH" "$LIMIT_HIGH"
sleep 2
$BENCHMARK
kill_pmg
mkdir -p scenario5
mv ./*.csv scenario5/

echo "=== All scenarios completed. CSV files are in scenario1..5 directories. ==="

