#!/bin/sh

if [ $# -lt 2 ]; then
    echo "Uso: $0 <directorio_coleccion> <dir_fotos1> [dir_fotos2 ...]" >&2
    exit 1
fi

COLLECTION="$1"
shift


for dir in "$@"; do
    if [ ! -d "$dir" ]; then
        echo "Error: El directorio '$dir' no existe." >&2
        exit 1
    fi
done


if [ -d "$COLLECTION" ]; then

    rm -rf "$COLLECTION"/* 2>/dev/null
else

    mkdir -p "$COLLECTION"
fi


OPS_FILE=$(mktemp)


find "$@" -type f \( \
    -name "*.[jJ][pP][gG]" -o -name "*.[jJ][pP][eE][gG]" -o \
    -name "*.[pP][nN][gG]" -o \
    -name "*.[tT][iI][fF][fF]" \
\) -print | while read -r filepath; do

    parent_dir=$(dirname "$filepath")
    

    parent_name=$(basename "$parent_dir")
    

    original_name=$(basename "$filepath")


    case "$original_name" in
        *.[jJ][pP][gG]|*.[jJ][pP][eE][gG]) ext=".jpg" ;;
        *.[pP][nN][gG])                    ext=".png" ;;
        *.[tT][iI][fF][fF])                ext=".tiff" ;;
        *) continue ;; 
    esac


    name_no_ext="${original_name%.*}"
    lower_name=$(echo "$name_no_ext" | tr 'A-Z' 'a-z')


    candidate="${parent_name}_${lower_name}${ext}"


    final_name=$(echo "$candidate" | tr ' ' '-')


    echo "${filepath}|${final_name}" >> "$OPS_FILE"

done

collisions=$(cut -d'|' -f2 "$OPS_FILE" | sort | uniq -d)

if [ -n "$collisions" ]; then
    echo "Error: Se han detectado colisiones en los nombres de fichero:" >&2
    echo "$collisions" >&2
    # "Terminar con error, dejando el directorio de la colección vacío"
    rm -rf "$COLLECTION"/* 2>/dev/null
    rm -f "$OPS_FILE"
    exit 1
fi


while IFS='|' read -r src dst; do
    cp "$src" "$COLLECTION/$dst"
done < "$OPS_FILE"

rm -f "$OPS_FILE"


cd "$COLLECTION" || exit 1

META_TMP=$(mktemp)
META_SORTED=$(mktemp)


ls -l | grep -v "^total" | grep -v "metadata.txt" | awk '{print $9, $5}' > "$META_TMP"


sort -k2 -n "$META_TMP" > "$META_SORTED"


awk '
    {
        print $0        # Imprimimos la línea (Nombre Tamaño)
        sum += $2       # Sumamos el tamaño
    }
    END {
        print "TOTAL:", sum, "bytes"
    }
' "$META_SORTED" > metadata.txt


rm -f "$META_TMP" "$META_SORTED"


exit 0