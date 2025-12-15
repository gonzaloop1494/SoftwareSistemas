#!/bin/bash

if [ -z "$1" ] || [ ! -d "$1" ]; then
    echo "Uso: $0 <directorio>"
    exit 1
fi

# Guardamos el directorio actual por si acaso (opcional)
# y entramos en el directorio destino. Si falla, salimos.
cd "$1" || exit 1

# Borrado silencioso (ya no necesitamos poner la ruta delante)
rm *.output 2>/dev/null

# Al estar dentro, el comodín *.txt ya nos da solo los nombres,
# no la ruta completa. ¡Ya no necesitamos basename ni recortes!
for filename in *.txt; do
    if [ -f "$filename" ]; then
        len=${#filename}
        
        # Concatenamos directamente al fichero de salida
        cat "$filename" >> "$len.output"
    fi
done

# (Opcional) Volver al directorio original si el script siguiera haciendo cosas
# cd - > /dev/null