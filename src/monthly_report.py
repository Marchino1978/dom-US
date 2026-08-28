import os
import csv
import base64
import traceback
import requests
from datetime import datetime, timedelta

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
    png_filename = f"report_{month_suffix}.png"
    
    csv_path = os.path.join(folder_path, csv_filename)
    png_path = os.path.join(folder_path, png_filename)
    
    print(f"Inizio generazione report per periodo: {start_date} -> {end_date}")
    
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

    labels = ["" for _ in rows]
    temperatures = [r.get("temperatura", 0) for r in rows]
    humidities = [r.get("umidita", 0) for r in rows]

    chart_config = {
        "type": "line",
        "data": {
            "labels": labels,
            "datasets": [
                {
                    "label": "TEMP (°C)",
                    "data": temperatures,
                    "borderColor": "red",
                    "backgroundColor": "rgba(255, 0, 0, 0.1)",
                    "fill": False,
                    "pointRadius": 0,
                    "borderWidth": 1.5
                },
                {
                    "label": "HUM (%)",
                    "data": humidities,
                    "borderColor": "blue",
                    "backgroundColor": "rgba(0, 0, 255, 0.1)",
                    "fill": False,
                    "pointRadius": 0,
                    "borderWidth": 1.5
                }
            ]
        },
        "options": {
            "title": {
                "display": True,
                "text": f"REPORT_{month_suffix.upper()}"
            },
            "legend": {
                "display": True,
                "position": "bottom"
            },
            "scales": {
                "xAxes": [{"display": False}],
                "yAxes": [{"display": True, "scaleLabel": {"display": True, "labelString": "Valori"}}]
            }
        }
    }

    try:
        qc_url = "https://quickchart.io/chart"
        qc_resp = requests.post(
            qc_url, 
            json={"chart": chart_config, "width": 900, "height": 500, "format": "png"}, 
            timeout=25
        )
        
        if qc_resp.status_code == 200:
            with open(png_path, "wb") as p_file:
                p_file.write(qc_resp.content)
            print(f"File PNG locale salvato: {png_path}")
        else:
            print(f"QuickChart errore {qc_resp.status_code}: {qc_resp.text}")
    except Exception as e:
        print(f"Errore generazione PNG: {e}")

    final_csv = csv_path if os.path.exists(csv_path) else None
    final_png = png_path if os.path.exists(png_path) else None

    return final_csv, final_png

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
        
        get_resp = requests.get(target_url, headers=headers, timeout=10)
        sha = get_resp.json().get("sha") if get_resp.status_code == 200 else None

        payload = {
            "message": "fix",
            "content": content,
            "branch": "main"
        }
        if sha:
            payload["sha"] = sha

        put_resp = requests.put(target_url, headers=headers, json=payload, timeout=10)
        
        if put_resp.status_code in [200, 201]:
            print(f"File {file_name} caricato con successo in data su GitHub.")
        else:
            print(f"Errore upload GitHub per {file_name}: {put_resp.text}")

    except Exception as e:
        print(f"Errore durante l'upload su GitHub: {e}")

if __name__ == "__main__":
    try:
        csv_p, png_p = generate_monthly_report()
        if csv_p:
            upload_to_github(csv_p)
        if png_p:
            upload_to_github(png_p)
        print("Esecuzione completata.")
    except Exception as e:
        print(f"Errore fatale: {e}")
        traceback.print_exc()
        exit(1)