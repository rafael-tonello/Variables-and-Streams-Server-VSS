#!/bin/bash

LOG_FILE="mem_log.dat"
PROCESS_NAME="vss"
REFRESH_INTERVAL=2

ctrlfile="/tmp/vssmemmonitor"
rm -f "$ctrlfile"

# Limpa o arquivo de log antes de começar
> "$LOG_FILE"

# Função de coleta de memória (em background)
collect_memory() {
    PID=$(pgrep "$PROCESS_NAME")
    if [ -z "$PID" ]; then
        echo "Processo '$PROCESS_NAME' não encontrado!"
        exit 1
    fi

    echo "Monitorando PID $PID ($PROCESS_NAME)..."

    while true; do
        if ps -p $PID > /dev/null; then
            MEM=$(ps -o rss= -p $PID --no-headers)
            TIMESTAMP=$(date +%s)
            echo "$TIMESTAMP $MEM" >> "$LOG_FILE"
        else
            echo "Processo terminou. Encerrando coleta."
            echo "done" >> "$ctrlfile"
            kill $$  # Encerra o script todo
            return 0
        fi
        sleep 1
    done
}

# Inicia coleta em background
collect_memory &
COLLECT_PID=$!

# Loop de exibição do gráfico no terminal
while kill -0 $COLLECT_PID 2>/dev/null; do
    clear
    echo "==============================="
    echo " Uso de Memória de $PROCESS_NAME (MB) - ASCII Plot"
    echo "==============================="
    echo

    if [ -s "$LOG_FILE" ]; then
        X_OFFSET=$(head -n 1 "$LOG_FILE" | awk '{print $1}')

        gnuplot -e "
            set terminal dumb size 120,30;
            set title 'Memória (MB)';
            set xlabel 'Tempo (s desde início)';
            set ylabel 'MB';
            set xrange [*:*];
            set yrange [*:*];
            plot '$LOG_FILE' using (\$1 - $X_OFFSET):(\$2/1024) with lines notitle;
        "

        # Mostrar o valor mais recente de memória (em MB)
        LAST_MEM_KB=$(tail -n 1 "$LOG_FILE" | awk '{print $2}')
        LAST_MEM_MB=$(echo "scale=2; $LAST_MEM_KB/1024" | bc)
        echo
        echo "Memória atual: ${LAST_MEM_MB} MB"
    else
        echo "(Nenhum dado coletado ainda...)"
    fi

    ctrlfiledata=$(cat "$ctrlfile" 2>/dev/null)
    if [ "$ctrlfiledata" == "done" ]; then
        echo "Coleta de memória finalizada."
        break
    fi

    sleep $REFRESH_INTERVAL
done

# Mata o coletor se ainda estiver rodando
kill $COLLECT_PID 2>/dev/null

rm -f "$ctrlfile"
rm -f "$LOG_FILE"
