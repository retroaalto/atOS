#!/bin/bash
cd "$(dirname "$0")"
rm -rf .git
git init -b main
git add .
git commit -m "Initial commit"
git status
chmod +x ./linux/*.sh
rm start.sh
rm start.bat