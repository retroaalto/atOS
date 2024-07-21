#!/bin/bash
cd "$(dirname "$0")"
cd z-linux
./build.sh
./run.sh "$@"