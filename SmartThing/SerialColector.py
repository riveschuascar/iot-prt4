import serial
import time
import os

# Configuración del puerto serial
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200
TIMEOUT = 1

TXT_FILE = "Resultados/datos_serial.txt"

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
        print(f"Conectado al puerto {SERIAL_PORT} ({BAUD_RATE} bps)")
    except serial.SerialException as e:
        print(f"Error al abrir el puerto serial: {e}")
        return

    # Crear la carpeta "Resultados" si no existe
    os.makedirs(os.path.dirname(TXT_FILE), exist_ok=True)

    with open(TXT_FILE, mode="a", encoding="utf-8") as file:
        print(f"Guardando datos en '{TXT_FILE}' (Ctrl+C para detener la captura)")

        try:
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode(errors="ignore").strip()
                    if line:
                        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
                        file.write(f"{timestamp} -> {line}\n")
                        file.flush()
                        print(f"{timestamp} -> {line}")
        except KeyboardInterrupt:
            print("\nCaptura detenida por el usuario.")
        finally:
            ser.close()
            print("Puerto serial cerrado.")

if __name__ == "__main__":
    main()
