#!/bin/sh

# ==============================================================================
# photocol.sh - Solución optimizada (Estilo Soriano/Guardiola)
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

# Ficheros temporales (usamos mktemp para seguridad, muy estilo Soriano)
# OPS_FILE guardará: "RutaOrigen|NombreDestino"
OPS_FILE=$(mktemp)
# ERR_FILE para comunicación entre subshell y proceso padre
ERR_FILE=$(mktemp)

# ==============================================================================
# FASE 1: PLANIFICACIÓN (Map)
# Buscamos y calculamos nombres, pero NO copiamos todavía.
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
        *) continue ;; # Por seguridad
    esac

    # Normalización de nombre
    name_no_ext="${original_name%.*}"
    lower_name=$(echo "$name_no_ext" | tr 'A-Z' 'a-z')
    candidate="${parent_name}_${lower_name}${ext}"
    final_name=$(echo "$candidate" | tr ' ' '-')

    # Guardamos la operación en el fichero temporal usando un separador seguro (|)
    echo "${filepath}|${final_name}" >> "$OPS_FILE"

done

# ==============================================================================
# FASE 2: DETECCIÓN DE COLISIONES (Reduce/Validate)
# ==============================================================================

# Extraemos solo los nombres de destino ($2), ordenamos y buscamos duplicados (-d)
# Si 'uniq -d' saca algo, es que hay colisión.
collisions=$(cut -d'|' -f2 "$OPS_FILE" | sort | uniq -d)

if [ -n "$collisions" ]; then
    echo "Error: Se han detectado colisiones en los nombres de fichero:" >&2
    echo "$collisions" >&2
    rm -rf "$COLLECTION"/* 2>/dev/null
    rm -f "$OPS_FILE" "$ERR_FILE"
    exit 1
fi

# ==============================================================================
# FASE 3: EJECUCIÓN (Execute)
# ==============================================================================

# Leemos el fichero de operaciones. IFS=| permite leer origen y destino
while IFS='|' read -r src dst; do
    cp "$src" "$COLLECTION/$dst"
done < "$OPS_FILE"

rm -f "$OPS_FILE" "$ERR_FILE"

# ==============================================================================
# FASE 4: METADATOS
# ==============================================================================

cd "$COLLECTION" || exit 1

# CORRECCIÓN CLAVE:
# 1. Usamos un fichero temporal fuera del pipe para evitar leerlo mientras escribimos.
# 2. ls -l puede variar según el sistema. Una forma más robusta en POSIX puro para
#    obtener tamaño y nombre es un poco compleja, pero usaremos ls -l asumiendo Linux.
#    Importante: awk '{print $9, $5}' asume columna 9=nombre, 5=tamaño.
#    Lo mandamos a un temporal PRIMERO.

# Modificación: Añadimos | grep -v "metadata" este es el cambio
ls -l | grep -v "^total" | grep -v "metadata" | awk '{print $9, $5}' | sort -k2 -n > metadata.tmp

# Calculamos el total y formateamos la salida final
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