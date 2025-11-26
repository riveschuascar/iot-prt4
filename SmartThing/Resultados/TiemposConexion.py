import re

INPUT_FILE = "Resultados/datos_serial.txt"
OUTPUT_FILE = "Resultados/tiempos_conexion_milisegundos.txt"

# Lista para almacenar los valores numéricos
valores = []

with open(INPUT_FILE, "r", encoding="utf-8") as infile, \
     open(OUTPUT_FILE, "w", encoding="utf-8") as outfile:
    
    for line in infile:
        # Buscar solo números al final de la línea
        match = re.search(r'\b\d+\b$', line.strip())
        if match:
            valor = int(match.group())
            valores.append(valor)
            outfile.write(str(valor) + "\n")

print(f"Archivo limpio guardado en '{OUTPUT_FILE}'")

# Calcular y mostrar la media
if valores:
    media = sum(valores) / len(valores)
    print(f"Cantidad de valores: {len(valores)}")
    print(f"Tiempo promedio (milisegundos): {media:.2f}")
else:
    print("No se encontraron valores numéricos para calcular la media.")
