#!/usr/bin/env bash
# Run once from NCamAndroid/ after cloning/extracting NCam source.
# Usage: ./setup_ncam_source.sh /path/to/NCam-master

set -e

NCAM_SRC="${1:-../NCam-master}"
CPP_DIR="app/src/main/cpp"
LINK="$CPP_DIR/ncam"

if [ ! -d "$NCAM_SRC" ]; then
    echo "ERROR: NCam source not found at $NCAM_SRC"
    exit 1
fi

# Symlink (preferred) or copy
if [ -L "$LINK" ]; then
    rm "$LINK"
fi
ln -s "$(realpath "$NCAM_SRC")" "$LINK"
echo "Linked $NCAM_SRC -> $LINK"

# Verify key files exist
for f in ncam.c globals.h config.h; do
    [ -f "$LINK/$f" ] || { echo "MISSING: $f"; exit 1; }
done

echo "Done. Now open the project in Android Studio."
