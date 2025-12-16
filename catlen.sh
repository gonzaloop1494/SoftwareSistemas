#!/bin/bash

if [ -z "$1" ] || [ ! -d "$1" ]; then
    echo "Uso: $0 <directorio>"
    exit 1
fi


cd "$1" || exit 1


rm *.output 2>/dev/null


for filename in *.txt; do
    if [ -f "$filename" ]; then
        len=${#filename}
        
        
        cat "$filename" >> "$len.output"
    fi
done

