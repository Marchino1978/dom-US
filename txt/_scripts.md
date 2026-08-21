# scripts//push.sh
----------------------------------------
#!/bin/bash

cd "$(dirname "$0")" || exit 1

cd ..

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

echo "➡️  Pull dal remoto (merge, no rebase)..."
git pull origin "$CURRENT_BRANCH" --no-rebase

git add --all -- :!backup_SQL/*

git commit -m "fix" 2>/dev/null || echo "ℹ️  Nessuna modifica da commitare"

echo "➡️  Push su branch: $CURRENT_BRANCH"
git push origin "$CURRENT_BRANCH"

# scripts//snapshot_all.sh
----------------------------------------
#!/bin/bash

mkdir -p txt

dump_folder() {
  local folder="$1"
  local output="txt/_$2.md"
  : > "$output"

  for file in "$folder"/*; do
    [ -f "$file" ] || continue

    local nome_file="${file##*/}"

# ESCLUDE FILE PER ESTENSIONE DALLO SNAPSHOT .md
    [[ "$nome_file" == *.zip ]] && continue

# ESCLUDE FILE SPECIFICI DALLO SNAPSHOT .md
    [[ "$nome_file" == "ETF.ino" ]] && continue
    [[ "$nome_file" == "frame.stl" ]] && continue
    [[ "$nome_file" == "README_FIRST.txt" ]] && continue
    [[ "$nome_file" == "README.md" ]] && continue
    [[ "$nome_file" == "TODO.md" ]] && continue

    echo "# $file" >> "$output"
    echo "----------------------------------------" >> "$output"

    cat "$file" >> "$output"

    echo "" >> "$output"
    echo "" >> "$output"
  done
}

dump_folder "." "root"

for dir in */; do
  [ -d "$dir" ] || continue
  foldername="${dir%/}"
  
  case "$foldername" in

# ESCLUDE INTERAMENTE QUESTE CARTELLE DALLO SNAPSHOT .md
    txt|.git|node_modules|data|public|old|gallery|img|backup_SQL|.venv|__pycache__) continue ;;
    *) dump_folder "$dir" "$foldername" ;;
  esac
done

# ESCLUSIONI FILE PER ESTENSIONE DAL TREE DEL PROGETTO
tree_exclude_files="*.zip"

# ESCLUSIONI CARTELLE DAL TREE DEL PROGETTO
tree_exclude_folders="node_modules|.git|txt"

# GENERA TREE DEL PROGETTO
tree -a -F -I "$tree_exclude_files|$tree_exclude_folders" --dirsfirst > project-tree.txt

echo "" >> project-tree.txt
echo "*** NOTE: some local files shown in this tree have been intentionally omitted from the repository ***" >> project-tree.txt

echo "Progetto mappato in project-tree.txt"
echo "Snapshot .md generati in txt"


# scripts//zippa.sh
----------------------------------------
#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

REPO_NAME=$(basename "$ROOT_DIR")
DATA_OGGI=$(date +%Y-%m-%d)
ZIP_NAME="${REPO_NAME}_backup_${DATA_OGGI}.zip"

echo "Avvio backup totale della repo: $REPO_NAME"
echo "Root del progetto individuata: $ROOT_DIR"

if ! command -v zip &> /dev/null; then
    echo "Errore: il comando 'zip' non è installato."
    echo "Installa con: sudo apt install zip"
    exit 1
fi

echo "Creazione dello zip (inclusi file nascosti, .env e .git)..."

zip -ry "$ZIP_NAME" . -x "*.zip"

echo "Backup completato con successo!"
echo "File creato nella root: $ROOT_DIR/$ZIP_NAME"


