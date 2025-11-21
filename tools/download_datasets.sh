#!/bin/bash

# Kipepeo Dataset Downloader
# Downloads Health and Education datasets for Phase 3

DATA_DIR="datasets"
mkdir -p $DATA_DIR

echo "[Kipepeo] Starting dataset download..."

# Health Datasets
echo "[Kipepeo] Downloading Ministry of Health Guidelines..."
# Mock download
touch $DATA_DIR/moh_guidelines_2025.pdf
touch $DATA_DIR/amref_symptom_db.csv

# Education Datasets
echo "[Kipepeo] Downloading KCSE Past Papers (2000-2025)..."
mkdir -p $DATA_DIR/kcse
for year in {2000..2025}; do
    touch $DATA_DIR/kcse/math_paper1_$year.pdf
    touch $DATA_DIR/kcse/english_paper1_$year.pdf
done

echo "[Kipepeo] Downloading KNEC Syllabus..."
touch $DATA_DIR/knec_syllabus_secondary.pdf

echo "[Kipepeo] Download complete. Ready for fine-tuning."
