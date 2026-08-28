import os
import base64
import requests
from datetime import datetime
from supabase import create_client

SUPABASE_URL = os.environ.get("SUPABASE_URL")
SUPABASE_KEY = os.environ.get("SUPABASE_KEY")
GITHUB_TOKEN = os.environ.get("GH_TOKEN")
GITHUB_REPO = "Marchino1978/dom-us"

tables_to_backup = ["sensor_data"]

def run_domus_backup():
    if not SUPABASE_URL or not SUPABASE_KEY:
        print("Errore: SUPABASE_URL o SUPABASE_KEY non trovati.")
        return None

    folder = "backup_SQL"
    filename = f"backup_supabase_{datetime.now().strftime('%Y_%m_%d')}.sql"
    file_path = os.path.join(folder, filename)
    
    print(f"Inizio generazione backup SQL per sensor_data: {filename}")
    
    try:
        supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
        os.makedirs(folder, exist_ok=True)
        
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(f"-- BACKUP AUTOMATICO DOM-US (sensor_data): {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            for table_name in tables_to_backup:
                resp = supabase.table(table_name).select("*").execute()
                rows = resp.data
                if not rows:
                    continue
                
                f.write(f"-- TABELLA: {table_name}\n")
                f.write(f"TRUNCATE TABLE {table_name};\n\n")
                
                for row in rows:
                    cols = ", ".join(row.keys())
                    vals_list = []
                    for v in row.values():
                        if v is None:
                            vals_list.append("NULL")
                        elif isinstance(v, (int, float)):
                            vals_list.append(str(v))
                        else:
                            safe_v = str(v).replace("'", "''")
                            vals_list.append(f"'{safe_v}'")
                    
                    vals_string = ", ".join(vals_list)
                    f.write(f"INSERT INTO {table_name} ({cols}) VALUES ({vals_string});\n")
                f.write("\n")
        
        print(f"Backup locale creato: {file_path}")
        return file_path
    except Exception as e:
        print(f"Errore generazione backup: {e}")
        return None

def upload_backup_to_github(file_path):
    if not GITHUB_TOKEN:
        print("Errore: GITHUB_TOKEN non trovato nelle variabili d'ambiente.")
        return

    headers = {"Authorization": f"Bearer {GITHUB_TOKEN}", "Accept": "application/vnd.github+json"}
    file_name = os.path.basename(file_path)
    api_url_base = f"https://api.github.com/repos/{GITHUB_REPO}/contents/backup_SQL"
    
    try:
        with open(file_path, "rb") as f:
            content = base64.b64encode(f.read()).decode("utf-8")
        
        put_resp = requests.put(f"{api_url_base}/{file_name}", headers=headers, json={
            "message": "fix",
            "content": content,
            "branch": "main"
        }, timeout=10)
        
        if put_resp.status_code in [200, 201]:
            print(f"Nuovo backup {file_name} caricato in backup_SQL su dom-us.")
        else:
            print(f"Errore upload GitHub: {put_resp.text}")

        # Rotazione backup (mantiene gli ultimi 3)
        resp = requests.get(api_url_base, headers=headers, timeout=10)
        if resp.status_code == 200:
            files = resp.json()
            backups = sorted([f for f in files if f['name'].endswith(".sql")], 
                            key=lambda x: x['name'], reverse=True)

            if len(backups) > 3:
                for old_file in backups[3:]:
                    del_url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/{old_file['path']}"
                    requests.delete(del_url, headers=headers, json={
                        "message": "fix",
                        "sha": old_file['sha'],
                        "branch": "main"
                    }, timeout=10)
                    print(f"Rimosso vecchio backup da GitHub: {old_file['name']}")

    except Exception as e:
        print(f"Errore durante la gestione backup su GitHub: {e}")

if __name__ == "__main__":
    path = run_domus_backup()
    if path: 
        upload_backup_to_github(path)