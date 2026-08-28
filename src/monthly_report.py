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
    
    year_str = first_day_prev_month.strftime("%Y")
    month_suffix = first_day_prev_month.strftime("%Y_%m")
    return start_str, end_str, year_str, month_suffix

def generate_monthly_report():
    start_date, end_date, year_str, month_suffix = get_previous_month_range()
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.dirname(base_dir) if os.path.basename(base_dir) == "scripts" else base_dir

    csv_folder = os.path.join(root_dir, "data", year_str, "csv")
    png_folder = os.path.join(root_dir, "data", year_str, "png")
    
    os.makedirs(csv_folder, exist_ok=True)
    os.makedirs(png_folder, exist_ok=True)
    
    csv_filename = f"report_{month_suffix}.csv"
    png_filename = f"report_{month_suffix}.png"
    
    csv_path = os.path.join(csv_folder, csv_filename)
    png_path = os.path.join(png_folder, png_filename)
    
    print(f"Generazione report per periodo: {start_date} -> {end_date}")
    
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
            print(f"Errore durante la query su Supabase: {e}")
            
    if not rows:
        print("Nessun dato trovato, inserisco riga vuota di fallback.")
        rows = [{"created_at": start_date, "temperatura": 0, "umidita": 0}]

    try:
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=["created_at", "temperatura", "umidita"])
            writer.writeheader()
            writer.writerows(rows)
        print(f"File CSV salvato con successo: {csv_path}")
    except Exception as e:
        print(f"Errore scrittura CSV: {e}")
        return None, None

    max_points = 500
    if len(rows) > max_points:
        step = len(rows) // max_points
        chart_rows = rows[::step]
    else:
        chart_rows = rows

    labels = ["" for _ in chart_rows]
    temperatures = [r.get("temperatura", 0) for r in chart_rows]
    humidities = [r.get("umidita", 0) for r in chart_rows]

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
        qc_resp = requests.post(qc_url, json={"chart": chart_config, "width": 900, "height": 500, "format": "png"}, timeout=20)
        
        if qc_resp.status_code == 200:
            with open(png_path, "wb") as p_file:
                p_file.write(qc_resp.content)
            print(f"File PNG salvato con successo: {png_path}")
        else:
            print(f"QuickChart ha risposto con codice {qc_resp.status_code}: {qc_resp.text}")
    except Exception as e:
        print(f"Timeout o errore generazione PNG: {e}")

    final_csv = csv_path if os.path.exists(csv_path) else None
    final_png = png_path if os.path.exists(png_path) else None

    return final_csv, final_png

def upload_to_github(file_path):
    if not GITHUB_TOKEN or not file_path or not os.path.exists(file_path):
        return False

    base_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.dirname(base_dir) if os.path.basename(base_dir) == "scripts" else base_dir
    rel_path = os.path.relpath(file_path, root_dir).replace("\\", "/")

    headers = {"Authorization": f"Bearer {GITHUB_TOKEN}", "Accept": "application/vnd.github+json"}
    api_url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/{rel_path}"
    
    try:
        with open(file_path, "rb") as f:
            content = base64.b64encode(f.read()).decode("utf-8")
        
        get_resp = requests.get(api_url, headers=headers, timeout=10)
        sha = get_resp.json().get("sha") if get_resp.status_code == 200 else None

        payload = {
            "message": "automated monthly report update",
            "content": content,
            "branch": "main"
        }
        if sha:
            payload["sha"] = sha

        put_resp = requests.put(api_url, headers=headers, json=payload, timeout=10)
        
        if put_resp.status_code in [200, 201]:
            print(f"File {rel_path} caricato correttamente su GitHub.")
            return True
        else:
            print(f"Errore upload GitHub per {rel_path}: {put_resp.text}")
            return False

    except Exception as e:
        print(f"Errore caricamento su GitHub {rel_path}: {e}")
        return False

if __name__ == "__main__":
    try:
        csv_p, png_p = generate_monthly_report()
        if csv_p:
            upload_to_github(csv_p)
        if png_p:
            upload_to_github(png_p)
        print("Esecuzione completata con successo.")
    except Exception as e:
        print(f"Errore fatale: {e}")
        traceback.print_exc()
        exit(1)