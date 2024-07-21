#!/bin/sh
echo "Running project..."
cd "$(dirname "$0")"
. ./scripts/globals.sh
cd ../build
./linux/"$PROJECTNAME" "$@"
cd ..