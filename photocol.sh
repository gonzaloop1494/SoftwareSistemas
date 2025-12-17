#!/bin/sh

# Función para mostrar uso y salir
usage() {
    echo "Uso: $0 <directorio_coleccion> <dir_fotos_1> [dir_fotos_2 ...]" >&2
    exit 1
}

# 1. Comprobación de argumentos (mínimo 2)
if [ "$#" -lt 2 ]; then
    usage
fi

COLLECTION_DIR="$1"
shift # Desplazamos para que $@ contenga solo los directorios origen

# 2. Comprobación de existencia de directorios origen
for dir in "$@"; do
    if [ ! -d "$dir" ]; then
        echo "Error: El directorio de origen '$dir' no existe." >&2
        exit 1
    fi
done

# 3. Preparación del directorio de la colección
if [ -d "$COLLECTION_DIR" ]; then
    # Si existe y no está vacío, borrar contenido.
    # Usamos find para borrar contenido recursivamente sin borrar la carpeta en sí,
    # o rm -rf * (pero * puede fallar si está vacío o hay muchos ficheros).
    # rm -rf es seguro aquí porque acabamos de verificar que es un directorio.
    rm -rf "${COLLECTION_DIR:?}/"*
else
    mkdir -p "$COLLECTION_DIR"
fi

# Obtenemos ruta absoluta de la colección para evitar problemas al hacer cp desde subdirectorios
# Como realpath no es siempre estándar en sh puro, usamos un truco con cd y pwd.
cd "$COLLECTION_DIR"
ABS_COLLECTION_DIR="$(pwd)"
cd - > /dev/null

# Ficheros temporales para control de flujo y metadata
# Usamos mktemp para seguridad (como les gusta a tus profesores)
META_TMP=$(mktemp)
ERROR_FLAG=$(mktemp)

# 4. Búsqueda y procesamiento de ficheros
# Buscamos en todos los directorios pasados como argumento ($@)
# -type f: solo ficheros
# \( ... \): agrupación de condiciones OR para las extensiones (case insensitive básico con globbing)
find "$@" -type f \( \
    -name "*.[jJ][pP][gG]" -o -name "*.[jJ][pP][eE][gG]" -o \
    -name "*.[pP][nN][gG]" -o \
    -name "*.[tT][iI][fF][fF]" -o -name "*.[tT][iI][fF]" \
    \) -print | while IFS= read -r filepath; do

    # a. Extraer información del path
    filename_full=$(basename "$filepath")
    dirname_parent=$(dirname "$filepath")
    parent_name=$(basename "$dirname_parent")
    
    # b. Identificar extensión original y definir la nueva normalizada
    # Usamos parameter expansion de sh: ${var##*.} para sacar extensión
    ext=".${filename_full##*.}"
    
    # Normalización de extensión (case insensitive manual)
    case "$ext" in
        *.([jJ][pP][gG]|.[jJ][pP][eE][gG])
            new_ext=".jpg"
            ;;
        *.([pP][nN][gG])
            new_ext=".png"
            ;;
        *.([tT][iI][fF]|.[tT][iI][fF][fF])
            new_ext=".tiff"
            ;;
        *.()
            # Si se colara algo por error, lo ignoramos
            continue
            ;;
    esac

    # c. Procesar nombre base (sin extensión)
    # ${filename_full%.*} elimina la extensión del string
    basename_raw="${filename_full%.*}"
    
    # Convertir a minúsculas (requisito del enunciado)
    basename_lower=$(echo "$basename_raw" | tr '[:upper:]' '[:lower:]')

    # d. Construir nombre candidato: Carpeta_Fichero.ext
    candidate_name="${parent_name}_${basename_lower}${new_ext}"

    # e. Reemplazar espacios por guiones en el nombre FINAL
    final_name=$(echo "$candidate_name" | tr ' ' '-')

    # f. Comprobar colisiones
    target_path="$ABS_COLLECTION_DIR/$final_name"
    
    if [ -e "$target_path" ]; then
        echo "Error: Colisión detectada. El fichero '$final_name' ya existe (origen: $filepath)." >&2
        touch "$ERROR_FLAG"
        break # Salimos del bucle while, pero ojo, estamos en una subshell por el pipe
    fi

    # g. Copiar fichero
    cp "$filepath" "$target_path"

    # h. Guardar info para metadata (Nombre Tamaño)
    # Usamos wc -c < fichero para obtener solo el número de bytes
    size=$(wc -c < "$target_path")
    echo "$final_name $size" >> "$META_TMP"

done

# 5. Verificación de errores post-bucle
# Como el while se ejecuta en una subshell (debido al pipe del find),
# las variables no persisten. Por eso usamos un fichero ERROR_FLAG.
if [ -f "$ERROR_FLAG" ] && [ -s "$ERROR_FLAG" ]; then
    rm "$ERROR_FLAG"
    rm "$META_TMP"
    # El enunciado dice: dejar el directorio vacío si hay error
    rm -rf "${COLLECTION_DIR:?}/"*
    exit 1
fi
rm "$ERROR_FLAG"

# 6. Generar metadata.txt
# El enunciado pide: Ordenado por tamaño (columna 2) numéricamente.
# Formato: Nombre Tamaño
if [ -s "$META_TMP" ]; then
    # Ordenamos por la 2a columna (-k2) numéricamente (-n)
    sort -k2 -n "$META_TMP" > "$ABS_COLLECTION_DIR/metadata.txt"

    # Calculamos el total con awk y lo añadimos al final
    # awk recorre el fichero ordenado, suma la col 2 y al final imprime el TOTAL
    awk '{sum += $2; print $0} END {print "TOTAL: " sum " bytes"}' "$ABS_COLLECTION_DIR/metadata.txt" > "$ABS_COLLECTION_DIR/metadata.txt.tmp" && mv "$ABS_COLLECTION_DIR/metadata.txt.tmp" "$ABS_COLLECTION_DIR/metadata.txt"
else
    # Si no hubo ficheros, creamos metadata vacío o con Total 0
    echo "TOTAL: 0 bytes" > "$ABS_COLLECTION_DIR/metadata.txt"
fi

rm "$META_TMP"

# Salida silenciosa si todo fue bien
exit 0