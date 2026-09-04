import os
import base64
import requests
from datetime import datetime, timedelta
import calendar
from supabase import create_client

SUPABASE_URL = os.environ.get("SUPABASE_URL")
SUPABASE_KEY = os.environ.get("SUPABASE_KEY")
GITHUB_TOKEN = os.environ.get("GH_TOKEN")
GITHUB_REPO = "Marchino1978/dom-US"

def get_previous_month_range():
    """Calcola inizio e fine del mese precedente in formato ISO (UTC)."""
    today = datetime.utcnow().date()
    first_day_this_month = today.replace(day=1)
    last_day_prev_month = first_day_this_month - timedelta(days=1)
    first_day_prev_month = last_day_prev_month.replace(day=1)
    
    start_str = f"{first_day_prev_month.strftime('%Y-%m-%d')}T00:00:00Z"
    end_str = f"{last_day_prev_month.strftime('%Y-%m-%d')}T23:59:59Z"
    
    month_suffix = first_day_prev_month.strftime("%Y_%m")
    return start_str, end_str, month_suffix

def run_logs_backup_and_cleanup():
    if not SUPABASE_URL or not SUPABASE_KEY:
        print("Errore: SUPABASE_URL o SUPABASE_KEY non trovati.")
        return None, None, None

    start_date, end_date, month_suffix = get_previous_month_range()
    folder = "backup_LOG"
    filename = f"backup_LOG_{month_suffix}.sql"
    file_path = os.path.join(folder, filename)
    
    print(f"Estrazione LOG per periodo: {start_date} -> {end_date}")
    print(f"Nome file backup: {filename}")
    
    try:
        supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
        os.makedirs(folder, exist_ok=True)
        
        resp = supabase.table("logs") \
            .select("*") \
            .gte("created_at", start_date) \
            .lte("created_at", end_date) \
            .execute()
        
        rows = resp.data
        if not rows:
            print(f"Nessun log trovato per il periodo {start_date} - {end_date}.")
            return None, None, None

        with open(file_path, "w", encoding="utf-8") as f:
            f.write(f"-- MONTHLY LOG BACKUP: {start_date} - {end_date}\n\n")
            f.write("-- TABLE: logs\n")
            
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
                f.write(f"INSERT INTO logs ({cols}) VALUES ({vals_string});\n")
        
        print(f"Backup locale LOG creato: {file_path}")
        return file_path, start_date, end_date

    except Exception as e:
        print(f"Errore durante l'estrazione log: {e}")
        return None, None, None

def upload_logs_to_github(file_path):
    if not GITHUB_TOKEN:
        print("Errore: GITHUB_TOKEN non trovato.")
        return False

    headers = {"Authorization": f"Bearer {GITHUB_TOKEN}", "Accept": "application/vnd.github+json"}
    file_name = os.path.basename(file_path)
    api_url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/backup_LOG/{file_name}"
    
    try:
        with open(file_path, "rb") as f:
            content = base64.b64encode(f.read()).decode("utf-8")
        
        put_resp = requests.put(api_url, headers=headers, json={
            "message": "fix",
            "content": content,
            "branch": "main"
        }, timeout=10)
        
        if put_resp.status_code in [200, 201]:
            print(f"Backup {file_name} caricato correttamente in /backup_LOG su GitHub.")
            return True
        else:
            print(f"Errore upload GitHub: {put_resp.text}")
            return False

    except Exception as e:
        print(f"Errore caricamento su GitHub: {e}")
        return False

def delete_logs_from_supabase(start_date, end_date):
    """Elimina i log da Supabase solo DOPO che il backup su GitHub è riuscito."""
    try:
        supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
        print(f"Pulizia tabella 'logs' da Supabase per intervallo {start_date} -> {end_date}...")
        
        supabase.table("logs") \
            .delete() \
            .gte("created_at", start_date) \
            .lte("created_at", end_date) \
            .execute()
            
        print("Cancellazione record su Supabase completata con successo!")
    except Exception as e:
        print(f"Errore durante l'eliminazione dei log da Supabase: {e}")

if __name__ == "__main__":
    path, start_d, end_d = run_logs_backup_and_cleanup()
    if path:
        success = upload_logs_to_github(path)
        # Cancella i dati da Supabase ESCLUSIVAMENTE se il file è stato salvato su GitHub
        if success:
            delete_logs_from_supabase(start_d, end_d)