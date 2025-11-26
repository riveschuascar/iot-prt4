# -*- coding: utf-8 -*-

import logging
import ask_sdk_core.utils as ask_utils
import boto3
import json

from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.dispatch_components import AbstractExceptionHandler
from ask_sdk_core.handler_input import HandlerInput
from ask_sdk_model import Response

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# AWS Clients
iot_client = boto3.client('iot-data', region_name='us-west-1')
dynamodb = boto3.resource('dynamodb', region_name='us-west-1')

# DynamoDB Table
USER_THING_TABLE = dynamodb.Table('user_thing')

# Global variable for IoT thing name
user_id = None
thing_name = None


def get_thing_name_from_user_id(user_id):
    try:
        response = USER_THING_TABLE.get_item(Key={'user_id': user_id})
        if 'Item' in response:
            thing_name = response['Item'].get('thing_name')
            logger.info(f"Thing encontrado para user_id {user_id}: {thing_name}")
            return thing_name
        else:
            logger.warning(f"No se encontró thing para user_id {user_id}")
            return None
    except Exception as e:
        logger.error(f"Error al consultar DynamoDB: {str(e)}")
        raise


def get_user_id_from_handler_input(handler_input):
    try:
        user_id = handler_input.request_envelope.context.system.user.user_id
        return user_id
    except Exception as e:
        logger.error(f"Error al obtener userId: {str(e)}")
        return None


class LaunchRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("LaunchRequest")(handler_input)

    def handle(self, handler_input):
        global thing_name, user_id
        try:
            user_id = get_user_id_from_handler_input(handler_input)
            if not user_id:
                speak_output = "No se pudo identificar tu usuario. Por favor intenta de nuevo."
                return handler_input.response_builder.speak(speak_output).ask(speak_output).response

            thing_name = get_thing_name_from_user_id(user_id)
            if not thing_name:
                speak_output = "No se encontró un dispositivo asociado a tu cuenta. Por favor contacta al administrador."
                return handler_input.response_builder.speak(speak_output).ask(speak_output).response

            logger.info(f"Sesión iniciada para user_id: {user_id}, thing_name: {thing_name}")
            speak_output = "Bienvenido al control de puerta y monitoreo de gas. Puedes decir abrir compuerta, cerrar compuerta, o consultar nivel de gas. ¿Qué deseas hacer?"
            reprompt_output = "Por favor, dime qué deseas hacer: abrir compuerta, cerrar compuerta, o consultar nivel de gas."

        except Exception as e:
            logger.error(f"Error en LaunchRequestHandler: {str(e)}")
            speak_output = "Hubo un error al inicializar la sesión. Por favor intenta de nuevo."
            reprompt_output = "Intenta de nuevo, ¿qué deseas hacer?"

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(reprompt_output)
                .response
        )


class OpenGateIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("OpenGateIntent")(handler_input)

    def handle(self, handler_input):
        global thing_name
        try:
            if not thing_name:
                speak_output = "Sesión no inicializada. Por favor vuelve a empezar."
                return handler_input.response_builder.speak(speak_output).ask(speak_output).response

            payload = {"state": {"desired": {"gate_state": "open"}}}
            iot_client.update_thing_shadow(thingName=thing_name, payload=json.dumps(payload))
            speak_output = "Abriendo la puerta. ¿Deseas hacer otra acción?"
            reprompt_output = "Puedes decir cerrar puerta o consultar nivel de gas."

        except Exception as e:
            logger.error(f"Error al abrir la puerta: {str(e)}")
            speak_output = "Hubo un error al intentar abrir la puerta. Por favor intenta de nuevo."
            reprompt_output = "Por favor dime qué deseas hacer."

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(reprompt_output)
                .response
        )


class CloseGateIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("CloseGateIntent")(handler_input)

    def handle(self, handler_input):
        global thing_name
        try:
            if not thing_name:
                speak_output = "Sesión no inicializada. Por favor vuelve a empezar."
                return handler_input.response_builder.speak(speak_output).ask(speak_output).response

            payload = {"state": {"desired": {"gate_state": "closed"}}}
            iot_client.update_thing_shadow(thingName=thing_name, payload=json.dumps(payload))
            speak_output = "Cerrando la puerta. ¿Deseas hacer otra acción?"
            reprompt_output = "Puedes decir abrir puerta o consultar nivel de gas."

        except Exception as e:
            logger.error(f"Error al cerrar la puerta: {str(e)}")
            speak_output = "Hubo un error al intentar cerrar la puerta. Por favor intenta de nuevo."
            reprompt_output = "Por favor dime qué deseas hacer."

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(reprompt_output)
                .response
        )


class CheckGasLevelIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("CheckGasLevelIntent")(handler_input)

    def handle(self, handler_input):
        global thing_name
        try:
            if not thing_name:
                speak_output = "Sesión no inicializada. Por favor vuelve a empezar."
                return handler_input.response_builder.speak(speak_output).ask(speak_output).response

            response = iot_client.get_thing_shadow(thingName=thing_name)
            shadow_data = json.loads(response['payload'].read())
            reported = shadow_data.get('state', {}).get('reported', {})
            gas_level_ppm = reported.get('gasLevel_ppm', 0)
            gas_level_state = reported.get('gasLevel_state', 'desconocido')

            if gas_level_state == "seguro":
                speak_output = f"El nivel de gas es seguro, con {gas_level_ppm} partes por millón."
            elif gas_level_state == "precaucion":
                speak_output = f"Precaución, el nivel de gas está en {gas_level_ppm} partes por millón. Mantente alerta."
            elif gas_level_state == "revisar":
                speak_output = f"Atención, se debe revisar el área. El nivel de gas está en {gas_level_ppm} partes por millón."
            elif gas_level_state == "evacuar":
                speak_output = f"¡Alerta! Se recomienda evacuar el área. El nivel de gas está en {gas_level_ppm} partes por millón."
            elif gas_level_state == "emergencia":
                speak_output = f"¡Emergencia! Evacúa inmediatamente. El nivel de gas está en {gas_level_ppm} partes por millón."
            else:
                speak_output = f"El nivel de gas es {gas_level_state}, con {gas_level_ppm} partes por millón."

            reprompt_output = "¿Deseas realizar otra acción?"

        except Exception as e:
            logger.error(f"Error al consultar el nivel de gas: {str(e)}")
            speak_output = "Hubo un error al intentar consultar el nivel de gas. Por favor intenta de nuevo."
            reprompt_output = "Por favor dime qué deseas hacer."

        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(reprompt_output)
                .response
        )


class HelpIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("AMAZON.HelpIntent")(handler_input)

    def handle(self, handler_input):
        speak_output = "Puedes decirme abrir puerta, cerrar puerta, o consultar nivel de gas. ¿Qué te gustaría hacer?"
        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(speak_output)
                .response
        )


class CancelOrStopIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return (ask_utils.is_intent_name("AMAZON.CancelIntent")(handler_input) or
                ask_utils.is_intent_name("AMAZON.StopIntent")(handler_input))

    def handle(self, handler_input):
        speak_output = "Adiós!"
        return (
            handler_input.response_builder
                .speak(speak_output)
                .response
        )


class SessionEndedRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("SessionEndedRequest")(handler_input)

    def handle(self, handler_input):
        return handler_input.response_builder.response


class CatchAllExceptionHandler(AbstractExceptionHandler):
    def can_handle(self, handler_input, exception):
        return True

    def handle(self, handler_input, exception):
        logger.error(exception, exc_info=True)
        speak_output = "Lo siento, tuve problemas haciendo lo que pediste. Por favor intenta de nuevo."
        return (
            handler_input.response_builder
                .speak(speak_output)
                .ask(speak_output)
                .response
        )


sb = SkillBuilder()
sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(OpenGateIntentHandler())
sb.add_request_handler(CloseGateIntentHandler())
sb.add_request_handler(CheckGasLevelIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_request_handler(SessionEndedRequestHandler())
sb.add_exception_handler(CatchAllExceptionHandler())

lambda_handler = sb.lambda_handler()
