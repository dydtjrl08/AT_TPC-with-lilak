#!/usr/bin/env bash
set -euo pipefail

# 사용 예:
#   ./run_tpc_ai_tutorial.sh data/run001_true.root data/run001_digi.root 42

TRUE_ROOT="${1:-data/run001_true.root}"
DIGI_ROOT="${2:-data/run001_digi.root}"
EVENT_ID="${3:-0}"

EVENT_CSV="${DIGI_ROOT%.*}_event.csv"
HIT_CSV="${DIGI_ROOT%.*}_hit.csv"
PAD_CSV="${DIGI_ROOT%.*}_pad.csv"

echo "[1/4] inspectSimFiles"
root -l -q "inspectSimFiles.C(\"${TRUE_ROOT}\",\"${DIGI_ROOT}\")"

echo "[2/4] exportTPCDToCSV"
root -l -q "exportTPCDToCSV.C(\"${DIGI_ROOT}\",\"${EVENT_CSV}\",\"${HIT_CSV}\",\"${PAD_CSV}\")"

echo "[3/4] CompareHTWithFittingAndAI_Heavy"
root -l -q "CompareHTWithFittingAndAI_Heavy.C(\"${DIGI_ROOT}\",\"${EVENT_CSV}\",${EVENT_ID})"

echo "[4/4] local baseline"
python3 tpcdrum_local_end_to_end.py --event_csv "${EVENT_CSV}"

echo "Done. outputs:"
echo "  - ${EVENT_CSV}"
echo "  - ${HIT_CSV}"
echo "  - ${PAD_CSV}"
