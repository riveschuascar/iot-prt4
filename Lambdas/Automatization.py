import boto3
import json
import os

# Obtener endpoint IoT desde Variables de entorno
IOT_ENDPOINT = os.environ.get("IOT_ENDPOINT")
iot = boto3.client("iot-data", endpoint_url=f"https://{IOT_ENDPOINT}")

def lambda_handler(event, context):
    print("Evento recibido:", json.dumps(event))

    thing_name = event.get("thing_name")
    ppm = event.get("gasLevel_ppm")

    if thing_name is None or ppm is None:
        print("Datos incompletos")
        return {"statusCode": 400, "body": "Missing data"}

    print(f"Nivel de ppm recibido: {ppm}")

    if ppm > 200:
        desired_servo = "open"
    else:
        desired_servo = "closed"

    print(f"Servo deseado: {desired_servo}")

    # Enviar actualización al Shadow
    payload = {
        "state": {
            "desired": {
                "gate_state": desired_servo
            }
        }
    }

    response = iot.update_thing_shadow(
        thingName=thing_name,
        payload=json.dumps(payload)
    )

    print("Shadow actualizado correctamente")
    return {"statusCode": 200, "body": "OK"}
