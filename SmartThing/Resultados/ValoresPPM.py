# Archivo de entrada
archivo_entrada = "SmartThing/Resultados/registro_ppm.txt"

# Archivo de salida
archivo_salida = "solo_ppm.txt"

ppm_values = []

# Leer archivo y extraer números
with open(archivo_entrada, "r") as f:
    for linea in f:
        if "PPM:" in linea:
            valor = linea.strip().split("PPM:")[1].strip()
            ppm_values.append(float(valor))  # convertir a float

# Guardar solo los valores en el archivo
with open(archivo_salida, "w") as f:
    for ppm in ppm_values:
        f.write(f"{ppm}\n")

# Calcular y mostrar la media
media_ppm = sum(ppm_values) / len(ppm_values)
print(f"Archivo con solo PPM generado: {archivo_salida}")
print(f"Media de PPM: {media_ppm:.2f}")
