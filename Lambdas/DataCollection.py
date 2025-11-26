import boto3
import json
import time

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('smoke_data')

def lambda_handler(event, context):
    print("Evento recibido:", json.dumps(event))

    # Datos enviados por la regla de IoT Core
    thing_name = event.get("thing_name")
    gas_level_ppm = event.get("gasLevel_ppm")
    gas_level_state = event.get("gasLevel_state")
    gate_state = event.get("gate_state")

    if gas_level_ppm is None:
        print("Nivel de gas no recibido")
        return {"statusCode": 400, "body": "No gas level received"}

    item = {
        "id": str(int(time.time())),
        "thing_name": thing_name,
        "gasLevel_ppm": gas_level_ppm,
        "gasLevel_state": gas_level_state,
        "gate_state": gate_state,
        "timestamp": int(time.time())
    }

    table.put_item(Item=item)
    print("Guardado en DynamoDB:", item)

    return {"statusCode": 200, "body": "OK"}