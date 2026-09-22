#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Configuration Paths
RESULTS_DIR="/home/simon/omnet-workspace/pifocd_v1/src/results"
SCAVETOOL="/home/simon/omnetpp-6.4.0/bin/opp_scavetool"
OUTPUT_CSV="lifetimes.csv"
DEST_DIR="/mnt/shared/"

echo "==> Navigating to results directory..."
cd "$RESULTS_DIR"

echo "==> Exporting vector results to CSV-R format..."
"$SCAVETOOL" export -f 'type =~ vector' -o "$OUTPUT_CSV" -F CSV-R *.vec

echo "==> Copying $OUTPUT_CSV to $DEST_DIR (requires root permissions)..."
sudo cp "$OUTPUT_CSV" "$DEST_DIR"

echo "==> Export and copy completed successfully!"
