#!/bin/bash

mkdir -p txt

dump_folder() {
  local folder="$1"
  local output="txt/_$2.md"
  : > "$output"

  find "$folder" -type f | while read -r file; do

    local nome_file="${file##*/}"

    # ESCLUDE FILE PER ESTENSIONE DALLO SNAPSHOT .md
    [[ "$nome_file" == *.zip ]] && continue

    # ESCLUDE FILE SPECIFICI DALLO SNAPSHOT .md
    [[ "$nome_file" == "README_FIRST.txt" ]] && continue
    [[ "$nome_file" == "README.md" ]] && continue
    [[ "$nome_file" == "TODO.md" ]] && continue
    [[ "$nome_file" == "config.h" ]] && continue

    echo "# $file" >> "$output"
    echo "----------------------------------------" >> "$output"

    cat "$file" >> "$output"

    echo "" >> "$output"
    echo "" >> "$output"
  done
}

local_output="txt/_root.md"
: > "$local_output"
for file in *; do
  if [ -f "$file" ]; then
    nome_file="${file##*/}"
    [[ "$nome_file" == "README.md" || "$nome_file" == "TODO.md" ]] && continue
    echo "# $file" >> "$local_output"
    echo "----------------------------------------" >> "$local_output"
    cat "$file" >> "$local_output"
    echo -e "\n\n" >> "$local_output"
  fi
done

for dir in */; do
  [ -d "$dir" ] || continue
  foldername="${dir%/}"
  
  case "$foldername" in
    # ESCLUDE INTERAMENTE QUESTE CARTELLE DALLO SNAPSHOT .md
    txt|.git|node_modules|data|public|old|stl|gallery|img|backup_SQL|backup_LOG|.venv|__pycache__) continue ;;
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
