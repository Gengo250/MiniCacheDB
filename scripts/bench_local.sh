#!/usr/bin/env bash
set -euo pipefail
PORT="${1:-6379}"

echo "[bench] Sending 50k SET + 50k GET via nc (rough baseline)."
TMP="$(mktemp)"
python3 - <<'PY' > "$TMP"
N = 50000
for i in range(N):
    print(f"SET k{i} v{i}")
for i in range(N):
    print(f"GET k{i}")
print("QUIT")
PY

START=$(date +%s%N)
cat "$TMP" | nc 127.0.0.1 "$PORT" > /dev/null
END=$(date +%s%N)

rm -f "$TMP"
NS=$((END-START))
MS=$((NS/1000000))
echo "[bench] Done in ~${MS} ms"
