#include <WiFi.h>
#include <PubSubClient.h>

//=====================
// DATOS WIFI
//=====================

const char* ssid = "ARROBO HALLO 💛";
const char* password = "Prilp@o2022";

// IP de la PC donde está Mosquitto
const char* mqtt_server = "192.168.0.105";

//=====================

WiFiClient espClient;
PubSubClient client(espClient);

#define LED_PIN 2
#define TMP36_PIN 34

bool ledState = false;
SemaphoreHandle_t xLedSemaphore;

//=====================
// CALLBACK MQTT
//=====================

void callback(char* topic, byte* payload, unsigned int length)
{
  String mensaje = "";

  for (int i = 0; i < length; i++)
    mensaje += (char)payload[i];

  Serial.print("Mensaje MQTT: ");
  Serial.println(mensaje);

  xSemaphoreTake(xLedSemaphore, portMAX_DELAY);

  if (mensaje == "ON")
    ledState = true;

  if (mensaje == "OFF")
    ledState = false;

  xSemaphoreGive(xLedSemaphore);
}

//=====================
// CONECTAR WIFI
//=====================

void conectarWiFi()
{
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Estado WiFi: ");
  Serial.println(WiFi.status());

  Serial.print("Modo WiFi: ");
  Serial.println(WiFi.getMode());

  Serial.print("Hostname: ");
  Serial.println(WiFi.getHostname());

  Serial.println("Probando conexión a la PC...");

  WiFiClient cliente;

  if (cliente.connect(IPAddress(192, 168, 0, 105), 1883)) {
    Serial.println("PC alcanzable en puerto 1883");
    cliente.stop();
  } else {
    Serial.println("No puedo conectar a la PC en el puerto 1883");
  }
}

//=====================
// CONECTAR MQTT
//=====================

void conectarMQTT()
{
  while (!client.connected())
  {
    Serial.println("Conectando MQTT...");

    if (client.connect("ESP32"))
    {
      Serial.println("MQTT conectado");
      client.subscribe("esp32/led");
    }
    else
    {
      Serial.println("Error conectando MQTT. Reintentando...");
      delay(2000);
    }
  }
}

//=====================
// TAREA MQTT
//=====================

void TaskMQTT(void *pvParameters)
{
  while (true)
  {
    if (!client.connected())
      conectarMQTT();

    client.loop();

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

//=====================
// TAREA SENSOR TMP36
//=====================

void TaskSensor(void *pvParameters)
{
  char dato[10];

  while (true)
  {
    // Lee directamente en milivoltios
    int milivoltios = analogReadMilliVolts(TMP36_PIN);

    // TMP36:
    // Temperatura °C = (mV - 500) / 10
    float temperatura = (milivoltios - 500) / 10.0;

    dtostrf(temperatura, 4, 1, dato);

    if (client.connected()) {
      client.publish("esp32/temperatura", dato);
    }

    Serial.print("Temperatura TMP36: ");
    Serial.print(dato);
    Serial.println(" °C");

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
//=====================
// TAREA LED
//=====================

void TaskLED(void *pvParameters)
{
  while (true)
  {
    xSemaphoreTake(xLedSemaphore, portMAX_DELAY);

    digitalWrite(LED_PIN, ledState);

    xSemaphoreGive(xLedSemaphore);

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

//=====================
// SETUP
//=====================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  analogReadResolution(12);
  analogSetPinAttenuation(TMP36_PIN, ADC_11db);

  conectarWiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  xLedSemaphore = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskMQTT, "MQTT", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskLED, "LED", 2048, NULL, 1, NULL, 1);
}

void loop()
{
}