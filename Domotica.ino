#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int PIN_SERVO  = 9;
const int PIN_BUZZER = 8;
const int PIN_CLK    = 7;
const int PIN_PIR    = 2;
const int PIN_TEMP   = A1;
const int PIN_LDR    = A0;

// Eliminamos el objeto Servo para liberar el Timer 1
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 1. Definición de las estructuras de datos para las colas
struct SensorData {
  int ldrValue;
  int pirValue;
  float tempValue;
};

struct ActionData {
  int servoAngle;
  bool alarmActive;
  bool ledActive;
  float tempValue;
  int ldrValue;
  int pirValue;
};

// 2. Declaración de las colas (Queues)
QueueHandle_t queueSensores;
QueueHandle_t queueAcciones;

// 3. Prototipos de las tareas
void TaskAdquisicion(void *pvParameters);
void TaskProcesamiento(void *pvParameters);
void TaskActuacion(void *pvParameters);

void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_CLK,    OUTPUT);
  pinMode(PIN_SERVO,  OUTPUT); // Configurado como salida manual
  pinMode(PIN_PIR,    INPUT);
  pinMode(PIN_LDR,    INPUT);

  // Posición inicial del servo manual (0 grados)
  digitalWrite(PIN_SERVO, LOW);
  
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando RTOS...");

  // Creación de las colas
  queueSensores = xQueueCreate(3, sizeof(SensorData));
  queueAcciones = xQueueCreate(3, sizeof(ActionData));

  // Verificar que las colas se crearon correctamente
  if (queueSensores != NULL && queueAcciones != NULL) {
    // 4. Creación de tareas con prioridades y STACKS optimizados para la RAM del Uno
    // Reducimos las tareas simples y le damos más espacio a la tarea que usa LCD/Serial
    xTaskCreate(TaskAdquisicion,    "Adquisicion",    85,  NULL, 3, NULL); // Prioridad Alta
    xTaskCreate(TaskProcesamiento, "Procesamiento",  85,  NULL, 2, NULL); // Prioridad Media
    xTaskCreate(TaskActuacion,     "Actuacion",     150, NULL, 1, NULL); // Prioridad Baja (Requiere más Stack)
  }
}

void loop() {
  // El loop vacío. FreeRTOS toma el control.
}

// -------------------------------------------------------------
// TAREA 1: Adquisición de Datos (Prioridad Alta)
// -------------------------------------------------------------
void TaskAdquisicion(void *pvParameters) {
  (void) pvParameters;
  SensorData data;
  
  for (;;) {
    data.ldrValue = analogRead(PIN_LDR);
    data.pirValue = digitalRead(PIN_PIR);
    
    int rawT = analogRead(PIN_TEMP);
    data.tempValue = (rawT * 5.0 * 100.0) / 1024.0;
    
    // Enviar datos a la cola
    xQueueSend(queueSensores, &data, portMAX_DELAY);
    
    // Muestreo cada 1000ms
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------------------------------------------------------------
// TAREA 2: Procesamiento (Prioridad Media)
// -------------------------------------------------------------
void TaskProcesamiento(void *pvParameters) {
  (void) pvParameters;
  SensorData receivedData;
  ActionData action;
  
  for (;;) {
    if (xQueueReceive(queueSensores, &receivedData, portMAX_DELAY) == pdPASS) {
      
      // Lógica de decisiones
      action.servoAngle = (receivedData.ldrValue > 500) ? 180 : 0;
      action.ledActive = (receivedData.pirValue == HIGH);
      action.alarmActive = (receivedData.tempValue <= 19.9 || receivedData.tempValue >= 31.0);
      
      // Traspaso de datos para la pantalla
      action.tempValue = receivedData.tempValue;
      action.ldrValue = receivedData.ldrValue;
      action.pirValue = receivedData.pirValue;
      
      xQueueSend(queueAcciones, &action, portMAX_DELAY);
    }
  }
}

// -------------------------------------------------------------
// TAREA 3: Actuación y Comunicación (Prioridad Baja)
// -------------------------------------------------------------
void TaskActuacion(void *pvParameters) {
  (void) pvParameters;
  ActionData receivedAction;
  int estadoServoActual = -1;
  
  for (;;) {
    if (xQueueReceive(queueAcciones, &receivedAction, portMAX_DELAY) == pdPASS) {
      
      // CONTROL DEL SERVO POR SOFTWARE (Evita el conflicto de Timers)
      if (estadoServoActual != receivedAction.servoAngle) {
        // Mapea el ángulo a microsegundos (0°->544us, 180°->2400us)
        int pulseWidth = map(receivedAction.servoAngle, 0, 180, 544, 2400);
        
        // Enviamos un tren de 20 pulsos para asegurar que el servo llegue a la posición
        for (int i = 0; i < 20; i++) {
          digitalWrite(PIN_SERVO, HIGH);
          delayMicroseconds(pulseWidth);
          digitalWrite(PIN_SERVO, LOW);
          
          // En lugar de bloquear el CPU con un delay largo, usamos vTaskDelay (20ms) 
          // para dejar que las otras tareas se ejecuten durante el periodo de refresco del servo
          vTaskDelay(pdMS_TO_TICKS(20)); 
        }
        estadoServoActual = receivedAction.servoAngle;
      }
      
      // Actuación del LED en PIN_CLK
      digitalWrite(PIN_CLK, receivedAction.ledActive ? HIGH : LOW);
      
      // Alarmas (Buzzer)
      digitalWrite(PIN_BUZZER, receivedAction.alarmActive ? HIGH : LOW);
      
      // Actualización de la Pantalla LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(receivedAction.tempValue, 1);
      lcd.print("C LDR:");
      lcd.print(receivedAction.ldrValue);
      
      lcd.setCursor(0, 1);
      lcd.print("PIR:");
      lcd.print(receivedAction.pirValue);
      lcd.print(" S:");
      lcd.print(estadoServoActual == 180 ? "ABI" : "CER");
      
      // Monitor Serial
      Serial.print("LDR: "); Serial.print(receivedAction.ldrValue);
      Serial.print(" | PIR: "); Serial.print(receivedAction.pirValue);
      Serial.print(" | TEMP: "); Serial.println(receivedAction.tempValue);
    }
  }
}
