#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <num_parallel_downloads> <csv files>..."
    exit 1
fi

JOBS="$1"
shift  # shift arguments so $@ is now the list of CSV files

for CSV_FILE in "$@"; do
    # Use filename (without extension) as album folder
    album_name=$(basename "$CSV_FILE" .csv)
    album_dir="downloads/$album_name"
    mkdir -p "$album_dir"

    echo ">>> Processing album: $album_name"

    # Skip header, parse each row
    tail -n +2 "$CSV_FILE" | while IFS=',' read -r uri track album artist rest; do
        # Strip quotes
        track=$(echo "$track" | sed 's/^"//; s/"$//')
        artist=$(echo "$artist" | sed 's/^"//; s/"$//')

        query="$track $artist"

        # Print command for GNU parallel
        echo "yt-dlp -x --audio-format mp3 -o \"$album_dir/%(title)s.%(ext)s\" \"ytsearch1:$query\""
    done
done | parallel -j"$JOBS"

