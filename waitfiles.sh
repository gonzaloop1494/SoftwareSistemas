#!/bin/sh

# Comprobar número de argumentos
if [ $# -eq 0 ]; then
    echo "Uso: $0 file1 [file2 ...]" >&2
    exit 1
fi

# Validación de ficheros
for f in "$@"; do
    if [ ! -f "$f" ]; then
        echo "Error: '$f' no existe o no es un fichero" >&2
        exit 1
    fi
done

# Polling hasta que todos sean eliminados
while true; do
    all_deleted=1

    for f in "$@"; do
        if [ -f "$f" ]; then
            all_deleted=0
            break
        fi
    done

    if [ "$all_deleted" -eq 1 ]; then
        echo "done"
        exit 0
    fi

    sleep 1
done
