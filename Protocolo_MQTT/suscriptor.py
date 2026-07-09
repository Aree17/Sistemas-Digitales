import paho.mqtt.client as mqtt

BROKER = "broker.hivemq.com"
PORT = 1883

TOPIC_TEMP = "laboratorio/temperatura"
TOPIC_LED = "laboratorio/led"


def on_connect(client, userdata, flags, rc):
    print("Conectado al broker")

    client.subscribe(TOPIC_TEMP)
    client.subscribe(TOPIC_LED)

    print("Suscrito a:")
    print("-", TOPIC_TEMP)
    print("-", TOPIC_LED)


def on_message(client, userdata, msg):
    print("--------------------------------")
    print("Tópico:", msg.topic)
    print("Mensaje:", msg.payload.decode())


client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

print("Conectando...")

client.connect(BROKER, PORT, 60)

client.loop_forever()
