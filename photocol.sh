#!/bin/sh

# ==============================================================================
# photocol.sh - Solución corregida
# ==============================================================================

if [ $# -lt 2 ]; then
    echo "Uso: $0 <directorio_coleccion> <dir_fotos1> [dir_fotos2 ...]" >&2
    exit 1
fi

COLLECTION="$1"
shift

# Comprobación de directorios origen
for dir in "$@"; do
    if [ ! -d "$dir" ]; then
        echo "Error: El directorio '$dir' no existe." >&2
        exit 1
    fi
done

# Preparación del directorio destino
if [ -d "$COLLECTION" ]; then
    rm -rf "$COLLECTION"/* 2>/dev/null
else
    mkdir -p "$COLLECTION"
fi

OPS_FILE=$(mktemp)
ERR_FILE=$(mktemp)

# ==============================================================================
# FASE 1: PLANIFICACIÓN (Map)
# ==============================================================================

find "$@" -type f \( \
    -name "*.[jJ][pP][gG]" -o -name "*.[jJ][pP][eE][gG]" -o \
    -name "*.[pP][nN][gG]" -o \
    -name "*.[tT][iI][fF][fF]" \
\) -print | while read -r filepath; do

    parent_dir=$(dirname "$filepath")
    parent_name=$(basename "$parent_dir")
    original_name=$(basename "$filepath")

    # Normalización de extensión
    case "$original_name" in
        *.[jJ][pP][gG]|*.[jJ][pP][eE][gG]) ext=".jpg" ;;
        *.[pP][nN][gG])                    ext=".png" ;;
        *.[tT][iI][fF][fF])                ext=".tiff" ;;
        *) continue ;; 
    esac

    # Normalización de nombre
    name_no_ext="${original_name%.*}"
    lower_name=$(echo "$name_no_ext" | tr 'A-Z' 'a-z')

    # --- CORRECCIÓN AQUÍ ---
    # Discriminamos: ¿Es nombre genérico (empieza por img) o descriptivo?
    case "$lower_name" in
        img*)
            # Es genérico (ej: img01), añadimos el prefijo de la carpeta
            candidate="${parent_name}_${lower_name}${ext}"
            ;;
        *)
            # Es descriptivo (ej: verano_playa), lo dejamos tal cual
            candidate="${lower_name}${ext}"
            ;;
    esac
    # -----------------------

    final_name=$(echo "$candidate" | tr ' ' '-')

    echo "${filepath}|${final_name}" >> "$OPS_FILE"

done

# ==============================================================================
# FASE 2: DETECCIÓN DE COLISIONES
# ==============================================================================

collisions=$(cut -d'|' -f2 "$OPS_FILE" | sort | uniq -d)

if [ -n "$collisions" ]; then
    echo "Error: Se han detectado colisiones en los nombres de fichero:" >&2
    echo "$collisions" >&2
    rm -rf "$COLLECTION"/* 2>/dev/null
    rm -f "$OPS_FILE" "$ERR_FILE"
    exit 1
fi

# ==============================================================================
# FASE 3: EJECUCIÓN
# ==============================================================================

while IFS='|' read -r src dst; do
    cp "$src" "$COLLECTION/$dst"
done < "$OPS_FILE"

rm -f "$OPS_FILE" "$ERR_FILE"

# ==============================================================================
# FASE 4: METADATOS
# ==============================================================================

cd "$COLLECTION" || exit 1

# Usamos ls -l asumiendo Linux estándar (col 5=size, col 9=name)
# El enunciado ordena por TAMAÑO (columna 2 del awk resultante), no por nombre.
ls -l | grep -v "^total" | grep -v "metadata" | awk '{print $9, $5}' | sort -k2 -n > metadata.tmp

awk '
    {
        sum += $2
        print $0
    }
    END {
        print "TOTAL:", sum, "bytes"
    }
' metadata.tmp > metadata.txt

rm -f metadata.tmp
exit 0