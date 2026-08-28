import os
import csv
import base64
import traceback
import requests
from datetime import datetime, timedelta

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

try:
    from supabase import create_client
except ImportError as e:
    print(f"ERRORE CRITICO: Modulo mancante -> {e}")
    exit(1)

SUPABASE_URL = os.environ.get("SUPABASE_URL")
SUPABASE_KEY = os.environ.get("SUPABASE_KEY")
GITHUB_TOKEN = os.environ.get("GH_TOKEN")
GITHUB_REPO = "Marchino1978/dom-us"

def get_previous_month_range():
    today = datetime.utcnow().date()
    first_day_this_month = today.replace(day=1)
    last_day_prev_month = first_day_this_month - timedelta(days=1)
    first_day_prev_month = last_day_prev_month.replace(day=1)
    
    start_str = f"{first_day_prev_month.strftime('%Y-%m-%d')}T00:00:00Z"
    end_str = f"{last_day_prev_month.strftime('%Y-%m-%d')}T23:59:59Z"
    month_suffix = first_day_prev_month.strftime("%Y_%m")
    return start_str, end_str, month_suffix

def generate_monthly_report():
    start_date, end_date, month_suffix = get_previous_month_range()
    
    folder = "data"
    current_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.dirname(current_dir)
    folder_path = os.path.join(root_dir, folder)
    
    os.makedirs(folder_path, exist_ok=True)
    
    csv_filename = f"report_{month_suffix}.csv"
    csv_path = os.path.join(folder_path, csv_filename)
    
    chart_filename = f"report_{month_suffix}.png"
    chart_path = os.path.join(folder_path, chart_filename)
    
    print(f"Inizio generazione report CSV e grafico PNG per periodo: {start_date} -> {end_date}")
    
    rows = []
    if SUPABASE_URL and SUPABASE_KEY:
        try:
            supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
            resp = supabase.table("sensor_data") \
                .select("created_at, temperatura, umidita") \
                .gte("created_at", start_date) \
                .lte("created_at", end_date) \
                .order("created_at", desc=False) \
                .execute()
            if hasattr(resp, 'data') and resp.data:
                rows = resp.data
            print(f"Query Supabase completata. Righe trovate: {len(rows)}")
        except Exception as e:
            print(f"Errore query Supabase: {e}")
            
    if not rows:
        print("Nessun dato trovato, inserisco riga di fallback.")
        rows = [{"created_at": start_date, "temperatura": 0, "umidita": 0}]

    try:
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=["created_at", "temperatura", "umidita"])
            writer.writeheader()
            writer.writerows(rows)
        print(f"File CSV locale salvato: {csv_path}")
    except Exception as e:
        print(f"Errore scrittura CSV: {e}")
        return None, None

    try:
        dates = []
        temperatures = []
        humidities = []
        
        for r in rows:
            dt_raw = r.get("created_at", "")
            try:
                dt = datetime.fromisoformat(dt_raw.replace("Z", "+00:00"))
            except Exception:
                try:
                    dt = datetime.strptime(dt_raw[:19], "%Y-%m-%d %H:%M:%S")
                except Exception:
                    dt = dt_raw
            dates.append(dt)
            temperatures.append(float(r.get("temperatura", 0) or 0))
            humidities.append(float(r.get("umidita", 0) or 0))

        plt.figure(figsize=(12, 6))
        plt.plot(dates, temperatures, label="Temperatura (°C)", color="#ff7f0e", linewidth=1.5)
        plt.plot(dates, humidities, label="Umidità (%)", color="#1f77b4", linewidth=1.5, alpha=0.8)
        
        plt.title(f"Report Sensori - {month_suffix}", fontsize=14, fontweight='bold')
        plt.xlabel("Data e Ora", fontsize=11)
        plt.ylabel("Valori", fontsize=11)
        plt.grid(True, linestyle="--", alpha=0.5)
        plt.legend(loc="upper left")
        plt.tight_layout()
        
        plt.savefig(chart_path, dpi=150)
        plt.close()
        print(f"File PNG locale salvato: {chart_path}")
    except Exception as e:
        print(f"Errore generazione grafico PNG locale: {e}")
        traceback.print_exc()

    return (
        csv_path if os.path.exists(csv_path) else None,
        chart_path if os.path.exists(chart_path) else None
    )

def upload_to_github(file_path):
    if not GITHUB_TOKEN or not file_path or not os.path.exists(file_path):
        return

    headers = {"Authorization": f"Bearer {GITHUB_TOKEN}", "Accept": "application/vnd.github+json"}
    file_name = os.path.basename(file_path)
    api_url_base = f"https://api.github.com/repos/{GITHUB_REPO}/contents/data"
    
    try:
        with open(file_path, "rb") as f:
            content = base64.b64encode(f.read()).decode("utf-8")
        
        target_url = f"{api_url_base}/{file_name}"
        
        get_resp = requests.get(target_url, headers=headers, timeout=30)
        sha = get_resp.json().get("sha") if get_resp.status_code == 200 else None

        payload = {
            "message": "fix",
            "content": content,
            "branch": "main"
        }
        if sha:
            payload["sha"] = sha

        put_resp = requests.put(target_url, headers=headers, json=payload, timeout=30)
        
        if put_resp.status_code in [200, 201]:
            print(f"File {file_name} caricato con successo in data su GitHub.")
        else:
            print(f"Errore upload GitHub per {file_name}: {put_resp.text}")

    except Exception as e:
        print(f"Errore durante l'upload su GitHub: {e}")

if __name__ == "__main__":
    try:
        csv_p, chart_p = generate_monthly_report()
        if csv_p:
            upload_to_github(csv_p)
        if chart_p:
            upload_to_github(chart_p)
        print("Esecuzione completata.")
    except Exception as e:
        print(f"Errore fatale: {e}")
        traceback.print_exc()
        exit(1)