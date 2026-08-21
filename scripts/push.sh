#!/bin/bash

cd "$(dirname "$0")" || exit 1

cd ..

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

echo "➡️  Pull dal remoto (merge, no rebase)..."
git pull origin "$CURRENT_BRANCH" --no-rebase

git add --all --

git commit -m "fix" 2>/dev/null || echo "ℹ️  Nessuna modifica da commitare"

echo "➡️  Push su branch: $CURRENT_BRANCH"
git push origin "$CURRENT_BRANCH"