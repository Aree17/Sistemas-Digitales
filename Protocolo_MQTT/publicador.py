import paho.mqtt.client as mqtt
import random
import time

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "laboratorio/temperatura"

client = mqtt.Client()

print("Conectando al broker...")
client.connect(BROKER, PORT, 60)

print("Conectado.")

while True:
    temperatura = round(random.uniform(20.0, 35.0), 1)

    client.publish(TOPIC, str(temperatura))

    print(f"[Python] Enviado: {temperatura} °C")

    time.sleep(2)
