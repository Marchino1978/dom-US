### `index.ts` (Edge Function)

```typescript
const SUPABASE_URL = Deno.env.get("SUPABASE_URL")!;
const SERVICE_ROLE_KEY = Deno.env.get("SB_SECRET_KEY")!;
const TELEGRAM_TOKEN = Deno.env.get("TELEGRAM_TOKEN")!;
const TELEGRAM_CHAT_ID = Deno.env.get("TELEGRAM_CHAT_ID")!;

const OFFLINE_THRESHOLD_SEC = 300;

async function sendTelegramMessage(text: string) {
  const url = `https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`;
  const resp = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      chat_id: TELEGRAM_CHAT_ID,
      text: text,
      parse_mode: "Markdown",
    }),
  });
  const respBody = await resp.text();
  console.log("Telegram response:", resp.status, respBody);
}

async function sendLogToSupabase(headers: Record<string, string>, createdAt: string, severity: string, eventMessage: string) {
  const resp = await fetch(`${SUPABASE_URL}/rest/v1/logs`, {
    method: "POST",
    headers,
    body: JSON.stringify({
      created_at: createdAt,
      severity: severity,
      event_message: eventMessage,
    }),
  });
  console.log("Supabase logs POST status:", resp.status);
}

function romeNowAsFakeUtc(): Date {
  const parts = new Intl.DateTimeFormat("en-GB", {
    timeZone: "Europe/Rome",
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: false,
  }).formatToParts(new Date());

  const get = (type: string) => parts.find((p) => p.type === type)!.value;
  const iso = `${get("year")}-${get("month")}-${get("day")}T${get("hour")}:${get("minute")}:${get("second")}Z`;
  return new Date(iso);
}

Deno.serve(async () => {
  const headers = {
    apikey: SERVICE_ROLE_KEY,
    Authorization: `Bearer ${SERVICE_ROLE_KEY}`,
    "Content-Type": "application/json",
  };

  const pingResp = await fetch(
    `${SUPABASE_URL}/rest/v1/device_status?id=eq.1&select=last_ping`,
    { headers }
  );
  const pingData = await pingResp.json();

  if (!pingData || pingData.length === 0) {
    return new Response("no device_status row", { status: 200 });
  }

  const lastPing = new Date(pingData[0].last_ping);
  const now = romeNowAsFakeUtc();
  const diffSec = (now.getTime() - lastPing.getTime()) / 1000;
  const isOffline = diffSec > OFFLINE_THRESHOLD_SEC;

  const statusResp = await fetch(
    `${SUPABASE_URL}/rest/v1/connection_status?id=eq.1&select=status`,
    { headers }
  );
  const statusData = await statusResp.json();

  if (!statusData || statusData.length === 0) {
    return new Response("no connection_status row", { status: 200 });
  }

  const currentStatus: boolean = statusData[0].status;

  if (isOffline && currentStatus === true) {
    await sendTelegramMessage(
      "🚫 *SYSTEM OFFLINE*\nconnection interrupted\nwi-fi or DSL problem"
    );

    await sendLogToSupabase(headers, now.toISOString(), "🔴", "🚫 SYSTEM OFFLINE");

    await fetch(`${SUPABASE_URL}/rest/v1/connection_status?id=eq.1`, {
      method: "PATCH",
      headers,
      body: JSON.stringify({ status: false }),
    });

    return new Response("transition to OFFLINE", { status: 200 });
  }

  if (!isOffline && currentStatus === false) {
    await sendTelegramMessage(
      "📶 *SYSTEM ONLINE*\nconnection restored"
    );

    await sendLogToSupabase(headers, now.toISOString(), "🟢", "📶 SYSTEM ONLINE");

    await fetch(`${SUPABASE_URL}/rest/v1/connection_status?id=eq.1`, {
      method: "PATCH",
      headers,
      body: JSON.stringify({ status: true }),
    });

    return new Response("transition to ONLINE", { status: 200 });
  }

  return new Response("no transition", { status: 200 });
});
```