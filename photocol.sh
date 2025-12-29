#!/bin/sh

# ==============================================================================
# photocol.sh - Solución Estricta según Enunciado
# ==============================================================================

# 1. Validación de argumentos (al menos destino y un origen)
if [ $# -lt 2 ]; then
    echo "Uso: $0 <directorio_coleccion> <dir_fotos1> [dir_fotos2 ...]" >&2
    exit 1
fi

COLLECTION="$1"
shift

# 2. Validación de existencia de directorios origen
for dir in "$@"; do
    if [ ! -d "$dir" ]; then
        echo "Error: El directorio '$dir' no existe." >&2
        exit 1
    fi
done

# 3. Preparación del directorio destino
if [ -d "$COLLECTION" ]; then
    # "Si ya existe y no está vacío, borrar todo"
    rm -rf "$COLLECTION"/* 2>/dev/null
else
    # "Si no existe, crear"
    mkdir -p "$COLLECTION"
fi

# Usamos ficheros temporales para gestionar la lista de copias y evitar
# problemas con pipes y subshells
OPS_FILE=$(mktemp)

# ==============================================================================
# FASE 1: BÚSQUEDA Y CÁLCULO DE NOMBRES
# ==============================================================================

# Buscamos recursivamente en todos los directorios pasados como argumentos
find "$@" -type f \( \
    -name "*.[jJ][pP][gG]" -o -name "*.[jJ][pP][eE][gG]" -o \
    -name "*.[pP][nN][gG]" -o \
    -name "*.[tT][iI][fF][fF]" \
\) -print | while read -r filepath; do

    # --- LÓGICA CRUCIAL DEL ENUNCIADO ---
    
    # 1. Obtener el directorio contenedor inmediato
    # Para "d3/verano/avion.PNG", parent_dir es "d3/verano"
    parent_dir=$(dirname "$filepath")
    
    # 2. Obtener "la última componente de su path"
    # De "d3/verano", sacamos "verano". De "d1", sacamos "d1".
    parent_name=$(basename "$parent_dir")
    
    # 3. Obtener nombre del fichero
    original_name=$(basename "$filepath")

    # 4. Normalización de extensión
    case "$original_name" in
        *.[jJ][pP][gG]|*.[jJ][pP][eE][gG]) ext=".jpg" ;;
        *.[pP][nN][gG])                    ext=".png" ;;
        *.[tT][iI][fF][fF])                ext=".tiff" ;;
        *) continue ;; # Por seguridad
    esac

    # 5. Nombre sin extensión y a minúsculas
    name_no_ext="${original_name%.*}"
    lower_name=$(echo "$name_no_ext" | tr 'A-Z' 'a-z')

    # 6. Construcción del candidato: DIRECTORIO + "_" + NOMBRE + EXT
    candidate="${parent_name}_${lower_name}${ext}"

    # 7. "Si los nombres tienen espacios, se cambian por -" en el nombre FINAL
    final_name=$(echo "$candidate" | tr ' ' '-')

    # Guardamos operación: ORIGEN | DESTINO
    echo "${filepath}|${final_name}" >> "$OPS_FILE"

done

# ==============================================================================
# FASE 2: DETECCIÓN DE COLISIONES
# ==============================================================================

# Comprobamos si hay nombres de destino repetidos
collisions=$(cut -d'|' -f2 "$OPS_FILE" | sort | uniq -d)

if [ -n "$collisions" ]; then
    echo "Error: Se han detectado colisiones en los nombres de fichero:" >&2
    echo "$collisions" >&2
    # "Terminar con error, dejando el directorio de la colección vacío"
    rm -rf "$COLLECTION"/* 2>/dev/null
    rm -f "$OPS_FILE"
    exit 1
fi

# ==============================================================================
# FASE 3: COPIA DE FICHEROS
# ==============================================================================

while IFS='|' read -r src dst; do
    cp "$src" "$COLLECTION/$dst"
done < "$OPS_FILE"

rm -f "$OPS_FILE"

# ==============================================================================
# FASE 4: GENERACIÓN DE METADATA.TXT
# ==============================================================================

cd "$COLLECTION" || exit 1

# CORRECCIÓN: Usamos ficheros temporales en /tmp con mktemp.
# Al estar fuera de la carpeta, 'ls -l' NO los verá y no saldrán en la lista.
META_TMP=$(mktemp)
META_SORTED=$(mktemp)

# 1. Generamos lista cruda: Nombre Tamaño (invertimos columnas de ls)
# Filtramos "metadata.txt" por si acaso se hubiera creado antes.
ls -l | grep -v "^total" | grep -v "metadata.txt" | awk '{print $9, $5}' > "$META_TMP"

# 2. Ordenamos por tamaño (columna 2), numérico (-n)
sort -k2 -n "$META_TMP" > "$META_SORTED"

# 3. Calculamos total y generamos fichero final
awk '
    {
        print $0        # Imprimimos la línea (Nombre Tamaño)
        sum += $2       # Sumamos el tamaño
    }
    END {
        print "TOTAL:", sum, "bytes"
    }
' "$META_SORTED" > metadata.txt

# Limpieza final de los temporales del sistema
rm -f "$META_TMP" "$META_SORTED"

# Salida limpia (sin mensajes)
exit 0