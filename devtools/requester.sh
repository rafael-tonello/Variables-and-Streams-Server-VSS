#!/usr/bin/env bash
set -u
# stress_vss.sh - simple stress/soak tester using curl, concurrency control and timeouts

# Config
#SERVER="http://localhost:5024"
SERVER="http://vss.antares:5024/n0/loadtest"
CONCURRENCY=20      # quantos requests paralelos por rodada
ROUNDS=0            # 0 -> loop infinito; >0 -> número de rodadas
REQUEST_TIMEOUT=5   # timeout do curl (segundos)
WORKER_TIMEOUT_MS=7000  # timeout que o script espera por cada worker (ms)
GET_PROB=80         # probabilidade (0-100) de executar GET em vez de POST
SLEEP_BETWEEN_ROUNDS_MS=150

# internal counters
totalSuccess=0
totalError=0
roundNo=0

# temp dir para arquivos de sinalização
TMPDIR=$(mktemp -d /tmp/stress_vss.XXXXXX)

#cleanup() {
#    rm -rf "$TMPDIR"
#    echo "Cleaned up. Exiting."
#}
#trap cleanup EXIT INT TERM

# worker: argument: worker-id
worker() {
    local id="$1"
    local rnd=$((RANDOM % 100 + 1))
    if [ "$rnd" -le "$GET_PROB" ]; then
        # GET
        if curl -sS --max-time "$REQUEST_TIMEOUT" "$SERVER/*" >/dev/null 2>&1; then
            echo "ok" >"$TMPDIR/requester_${id}"
            return 0
        else
            echo "erro" >"$TMPDIR/requester_${id}"
            return 1
        fi
    else
        # POST
        local payload="$RANDOM"
        if curl -sS --max-time "$REQUEST_TIMEOUT" -X POST "$SERVER/n0/serverTest/$RANDOM" -d "$payload" >/dev/null 2>&1; then
            echo "ok" >"$TMPDIR/requester_${id}"
            return 0
        else
            echo "erro" >"$TMPDIR/requester_${id}"
            return 1
        fi
    fi
}

# wait_for_worker_file: waits until file exists or timeout; returns 0 if file exists, 1 if timeout
wait_for_worker_file() {
    local file="$1"
    local start_ms
    local now_ms
    start_ms=$(date +%s%3N)
    while [ ! -f "$file" ]; do
        sleep 0.05
        now_ms=$(date +%s%3N)
        elapsed=$((now_ms - start_ms))
        if [ "$elapsed" -gt "$WORKER_TIMEOUT_MS" ]; then
            return 1
        fi
    done
    return 0
}

# main loop
while : ; do
    roundNo=$((roundNo + 1))
    if [ "$ROUNDS" -ne 0 ] && [ "$roundNo" -gt "$ROUNDS" ]; then
        break
    fi

    # limpa estado antigo
    rm -f "$TMPDIR"/requester_*
    echo "=== Round $roundNo: spawning $CONCURRENCY workers ==="

    # iniciar workers em background
    pids=()
    for ((i=1; i<=CONCURRENCY; i++)); do
        worker "$i" &   # cada worker gravará "$TMPDIR/requester_$i"
        pids+=("$!")
    done

    # esperar cada worker com timeout (por arquivo criado)
    successCount=0
    errorCount=0
    for ((i=1; i<=CONCURRENCY; i++)); do
        file="$TMPDIR/requester_${i}"
        if wait_for_worker_file "$file"; then
            result=$(cat "$file")
            if [ "$result" = "ok" ]; then
                successCount=$((successCount+1))
            else
                errorCount=$((errorCount+1))
            fi
        else
            # timeout: tentar matar o processo (se ainda existir) para liberar recursos
            # procurar PID relacionado ao worker (pids array)
            pid="${pids[$((i-1))]}"
            if kill -0 "$pid" >/dev/null 2>&1; then
                kill "$pid" >/dev/null 2>&1 || true
            fi
            echo "timeout -> marking erro for worker $i"
            echo "erro" >"$file"
            errorCount=$((errorCount+1))
        fi
    done

    totalSuccess=$((totalSuccess + successCount))
    totalError=$((totalError + errorCount))
    total=$((totalSuccess + totalError))

    if [ "$total" -gt 0 ]; then
        # calcular taxa com bc
        successRate=$(echo "scale=2; ($totalSuccess / $total) * 100" | bc)
    else
        successRate="0.00"
    fi

    echo "Round $roundNo done. This round -> success:$successCount error:$errorCount. All -> success:$totalSuccess error:$totalError total:$total successRate:${successRate}%"
    # pequena pausa
    sleep "$(awk "BEGIN{print $SLEEP_BETWEEN_ROUNDS_MS/1000}")"
done
