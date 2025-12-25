#!/bin/sh

if test $# -lt 2
then
    echo "$0 collection dir1 [dir2 ... dirN]" 1>&2
    exit 1
fi

collection="$1"
shift

# Preparación del directorio colección
if ! test -d "$collection"
then
    mkdir -p "$collection"
else
    # Usamos ls -A para ver si hay algo (incluso ocultos) y evitar errores si está vacío
    if test -n "$(ls -A "$collection" 2>/dev/null)"
    then
        rm -rf "$collection"/*
    fi
fi

# Verificación de directorios origen
for d in "$@"
do
    if ! test -d "$d"
    then
        echo "$d no dir" 1>&2
        exit 1
    fi
done

tmp=$(mktemp)
tmpmeta=$(mktemp)

# Búsqueda corregida: comillas y paréntesis
for d in "$@"
do
    find "$d" -type f \( -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o -iname "*.tiff" \)
done > "$tmp"

# Función de limpieza por error
die() {
    echo "Error: $1" >&2
    rm -rf "$collection"/*
    rm -f "$tmp" "$tmpmeta"
    exit 1
}

while IFS= read -r line
do
    # 1. Obtener nombre base del fichero
    filename_orig=$(basename "$line")
    
    # 2. Obtener extensión y nombre sin extensión
    # La sintaxis ${var##*.} es estándar en sh/bash para extensiones, pero usaremos sed por compatibilidad básica
    ext=$(echo "$filename_orig" | sed -E 's/.*\.([a-zA-Z0-9]+)$/\1/' | tr '[:upper:]' '[:lower:]')
    name_no_ext=$(echo "$filename_orig" | sed -E 's/(.*)\.[a-zA-Z0-9]+$/\1/')

    # 3. Obtener nombre del directorio padre
    dir_path=$(dirname "$line")
    dir_name=$(basename "$dir_path")

    # 4. Normalizar nombres (espacios -> guiones, todo minúsculas)
    # NOTA: El enunciado pide espacios -> guiones (-), no guiones bajos (_)
    clean_dir=$(echo "$dir_name" | tr '[:upper:]' '[:lower:]' | tr ' ' '-')
    clean_name=$(echo "$name_no_ext" | tr '[:upper:]' '[:lower:]' | tr ' ' '-')

    # 5. Normalizar extensión jpeg -> jpg
    if test "$ext" = "jpeg"
    then
        ext="jpg"
    fi

    # 6. Construir nombre final: DIRECTORIO_FICHERO.EXT
    newfile="${clean_dir}_${clean_name}.${ext}"
    dest_path="$collection/$newfile"

    # 7. DETECTAR COLISIÓN
    if test -e "$dest_path"
    then
        die "Colisión detectada: $newfile ya existe."
    fi
    
    # Copiar
    cp "$line" "$dest_path"
    
    # Obtener tamaño (stat es mejor, pero ls -l awk suele funcionar)
    # En Linux estándar:
    size=$(stat -c%s "$dest_path")
    # Si no tienes stat, tu método ls -l funciona casi siempre:
    # size=$(ls -l "$dest_path" | awk '{print $5}')
    
    echo "$newfile $size" >> "$tmpmeta"
    
done < "$tmp"

# Ordenar metadata
sort -k2 -n "$tmpmeta" > "$collection"/metadata.txt

# Calcular total
awk '{s=s+$2} END{print "TOTAL: "s " bytes"}' "$tmpmeta" >> "$collection"/metadata.txt

# Limpieza
rm -f "$tmp" "$tmpmeta"