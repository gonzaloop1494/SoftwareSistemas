#!/bin/sh

# ==============================================================================
# photocol.sh - Generador de colección de fotos
# Metodología: GSYC (Sistemas Operativos)
# ==============================================================================

# 1. Validación de argumentos
# Se requieren al menos 2 argumentos: directorio colección y al menos 1 fuente.
if [ $# -lt 2 ]; then
    echo "Uso: $0 <directorio_coleccion> <dir_fotos1> [dir_fotos2 ...]" >&2
    exit 1
fi

COLLECTION="$1"
shift # Desplaza los argumentos para que $@ contenga solo los directorios fuente

# 2. Validación de directorios fuente
# Recorremos los argumentos restantes para asegurar que existen.
for dir in "$@"; do
    if [ ! -d "$dir" ]; then
        echo "Error: El directorio '$dir' no existe." >&2
        exit 1
    fi
done

# 3. Preparación del directorio de la colección
# Si existe, borramos contenido. Si no, lo creamos.
if [ -d "$COLLECTION" ]; then
    # Usamos rm -rf sobre el contenido, ocultos incluidos si los hubiera.
    rm -rf "$COLLECTION"/* 2>/dev/null
else
    mkdir -p "$COLLECTION"
fi

# Fichero temporal para detectar errores dentro del subshell del pipe
ERROR_FILE="$COLLECTION/.fatal_error"

# 4. Búsqueda y Procesamiento (Pipeline Principal)
# Usamos 'find' para buscar recursivamente en todos los directorios pasados ($@).
# Filtramos por extensiones (case-insensitive) usando -name con OR.
find "$@" -type f \( \
    -name "*.[jJ][pP][gG]" -o -name "*.[jJ][pP][eE][gG]" -o \
    -name "*.[pP][nN][gG]" -o \
    -name "*.[tT][iI][fF][fF]" \
\) -print | while read -r filepath; do

    # --- Extracción de partes del path ---
    # Obtenemos el nombre del directorio padre (última componente del path)
    parent_dir=$(dirname "$filepath")
    parent_name=$(basename "$parent_dir")
    
    # Obtenemos el nombre del fichero original
    original_name=$(basename "$filepath")

    # --- Normalización de la extensión ---
    # Usamos case para determinar la extensión correcta según el enunciado.
    case "$original_name" in
        *.[jJ][pP][gG]|*.[jJ][pP][eE][gG])
            ext=".jpg"
            ;;
        *.[pP][nN][gG])
            ext=".png"
            ;;
        *.[tT][iI][fF][fF])
            ext=".tiff"
            ;;
        *)
            # Si se coló algo raro (improbable por el find), saltamos
            continue
            ;;
    esac

    # --- Normalización del nombre ---
    # 1. Quitamos la extensión original.
    #    La expansión ${var%.*} elimina desde el último punto hacia el final.
    name_no_ext="${original_name%.*}"

    # 2. Convertimos el nombre del fichero a minúsculas.
    lower_name=$(echo "$name_no_ext" | tr 'A-Z' 'a-z')

    # 3. Construimos el nombre candidato (con espacios aún).
    candidate="${parent_name}_${lower_name}${ext}"

    # 4. Sustituimos espacios por guiones en el nombre FINAL completo.
    final_name=$(echo "$candidate" | tr ' ' '-')

    # --- Detección de Colisiones ---
    target_path="$COLLECTION/$final_name"
    
    if [ -e "$target_path" ]; then
        echo "Error: Colisión detectada para el fichero '$final_name'." >&2
        touch "$ERROR_FILE"
        break # Salimos del bucle while
    fi

    # --- Copia del fichero ---
    cp "$filepath" "$target_path"

done

# 5. Verificación de errores post-bucle
# Como el while se ejecuta en un subshell (por el pipe), las variables
# no persisten. Comprobamos si se creó el fichero de error.
if [ -f "$ERROR_FILE" ]; then
    rm -f "$ERROR_FILE"
    # El enunciado dice: "dejando el directorio de la colección vacío"
    rm -rf "$COLLECTION"/*
    exit 1
fi

# 6. Generación de Metadata (Filtros de texto)
# Requisito: Nombre y tamaño, ordenado por tamaño, y total al final.
# Entramos al directorio para que ls nos de nombres limpios sin ruta.
cd "$COLLECTION" || exit 1

# Explicación del pipeline:
# 1. ls -l: Lista ficheros con detalles (tamaño suele ser columna 5).
# 2. awk: Filtra la línea 'total' de ls, e imprime "Nombre Tamaño".
#    NOTA: Usamos awk para reordenar porque ls -l da el tamaño antes del nombre.
# 3. sort: Ordena numéricamente (-n) por la segunda columna (tamaño) (-k2).
# 4. Redirigimos a un temporal para calcular el total después sin ensuciar la lectura.

ls -l | grep -v "^total" | awk '{print $9, $5}' | sort -n -k2 > metadata.txt

# Calculamos el total usando awk sobre el fichero ya generado y lo añadimos.
# Usamos cat para alimentar a awk (estilo clásico de filtros).
cat metadata.txt | awk '
    {
        sum += $2;      # Sumamos la segunda columna (tamaño)
        print $0;       # Imprimimos la línea tal cual (ya está ordenada)
    }
    END {
        print "TOTAL:", sum, "bytes"
    }
' > metadata.final

# Reemplazamos el metadata.txt con la versión final (con el total)
mv metadata.final metadata.txt

# Salimos con éxito (sin output por pantalla)
exit 0