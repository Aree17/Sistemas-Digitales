const byte PIN_TEMP = A0;
const byte PIN_BOTON = 2;
const byte LED_VERDE = 8;
const byte LED_ROJO = 7;
const byte BUZZER = 6;

const unsigned long PERIODO_HEARTBEAT = 500;
const unsigned long PERIODO_SENSOR = 200;
const unsigned long PERIODO_TELEMETRIA = 2000;
const unsigned long PERIODO_ALARMA = 300;
const unsigned long PERIODO_DEBOUNCE = 50;

const float TEMP_ACTIVAR_ALARMA = 30.0;
const float TEMP_DESACTIVAR_ALARMA = 30.0;

unsigned long tiempoHeartbeat = 0;
unsigned long tiempoSensor = 0;
unsigned long tiempoTelemetria = 0;
unsigned long tiempoAlarma = 0;
unsigned long tiempoBoton = 0;

float temperatura = 0.0;

bool estadoLedVerde = LOW;
bool estadoAlarmaVisual = LOW;
bool botonAnterior = LOW;

enum EstadoSistema {
  NORMAL,
  ALARMA,
  SILENCIADA
};

EstadoSistema estadoActual = NORMAL;

float leerTemperaturaTMP36() {
  long sumaLecturas = 0;
  const int NUM_MUESTRAS = 10;

  for (int i = 0; i < NUM_MUESTRAS; i++) {
    sumaLecturas += analogRead(PIN_TEMP);
  }

  float lecturaPromedio = sumaLecturas / float(NUM_MUESTRAS);
  float voltaje = lecturaPromedio * (5.0 / 1023.0);
  float temperaturaC = (voltaje - 0.5) * 100.0;

  return temperaturaC;
}

void apagarAlarma() {
  digitalWrite(LED_ROJO, LOW);
  noTone(BUZZER);
  estadoAlarmaVisual = LOW;
}

const char* obtenerNombreEstado() {
  switch (estadoActual) {
    case NORMAL:
      return "NORMAL";

    case ALARMA:
      return "ALARMA";

    case SILENCIADA:
      return "SILENCIADA";

    default:
      return "DESCONOCIDO";
  }
}

void actualizarTemperatura(unsigned long tiempoActual) {
  if (tiempoActual - tiempoSensor >= PERIODO_SENSOR) {
    tiempoSensor = tiempoActual;
    temperatura = leerTemperaturaTMP36();
  }
}

void tareaHeartbeat(unsigned long tiempoActual) {
  if (tiempoActual - tiempoHeartbeat >= PERIODO_HEARTBEAT) {
    tiempoHeartbeat = tiempoActual;

    estadoLedVerde = !estadoLedVerde;
    digitalWrite(LED_VERDE, estadoLedVerde);
  }
}

void tareaTelemetria(unsigned long tiempoActual) {
  if (tiempoActual - tiempoTelemetria >= PERIODO_TELEMETRIA) {
    tiempoTelemetria = tiempoActual;

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Estado: ");
    Serial.println(obtenerNombreEstado());

    if (estadoActual == ALARMA) {
      Serial.println("ALERTA: Temperatura mayor a 30 °C");
    }

    if (estadoActual == SILENCIADA) {
      Serial.println("Alarma silenciada manualmente");
    }

    if (estadoActual == NORMAL) {
      Serial.println("Sistema en estado normal");
    }

    Serial.println("--------------------------------");
  }
}

void gestionarFSM() {
  switch (estadoActual) {
    case NORMAL:
      if (temperatura > TEMP_ACTIVAR_ALARMA) {
        estadoActual = ALARMA;
        Serial.println("Cambio de estado: NORMAL -> ALARMA");
      }
    break;

    case ALARMA:
      if (temperatura <= TEMP_DESACTIVAR_ALARMA) {
        estadoActual = NORMAL;
        apagarAlarma();
        Serial.println("Cambio de estado: ALARMA -> NORMAL");
      }
    break;

    case SILENCIADA:
      if (temperatura <= TEMP_DESACTIVAR_ALARMA) {
        estadoActual = NORMAL;
        apagarAlarma();
        Serial.println("Cambio de estado: SILENCIADA -> NORMAL");
      }
    break;
  }
}

void tareaBoton(unsigned long tiempoActual) {
  bool botonActual = digitalRead(PIN_BOTON);

  if (tiempoActual - tiempoBoton >= PERIODO_DEBOUNCE) {
    tiempoBoton = tiempoActual;

    if (botonActual == HIGH && botonAnterior == LOW) {
      if (estadoActual == ALARMA) {
        estadoActual = SILENCIADA;
        apagarAlarma();
        Serial.println("ACK: Alarma silenciada por el operador");
      } 
      else if (estadoActual == SILENCIADA && temperatura > TEMP_ACTIVAR_ALARMA) {
        estadoActual = ALARMA;
        Serial.println("ACK: Alarma reactivada por el operador");
      }
    }

    botonAnterior = botonActual;
  }
}

void tareaAlarma(unsigned long tiempoActual) {
  if (estadoActual == ALARMA) {
    if (tiempoActual - tiempoAlarma >= PERIODO_ALARMA) {
      tiempoAlarma = tiempoActual;

      estadoAlarmaVisual = !estadoAlarmaVisual;
      digitalWrite(LED_ROJO, estadoAlarmaVisual);

      if (estadoAlarmaVisual == HIGH) {
        tone(BUZZER, 1000);
      } 
      else {
        noTone(BUZZER);
      }
    }
  } 
  else {
    apagarAlarma();
  }
}

void setup() {
  pinMode(PIN_TEMP, INPUT);
  pinMode(PIN_BOTON, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);
  noTone(BUZZER);

  Serial.begin(9600);

  Serial.println("================================");
  Serial.println("SISTEMA IoT INICIADO");
  Serial.println("Sensor TMP36");
  Serial.println("FSM + millis()");
  Serial.println("================================");
}

void loop() {
  unsigned long tiempoActual = millis();

  actualizarTemperatura(tiempoActual);
  tareaHeartbeat(tiempoActual);
  tareaTelemetria(tiempoActual);
  gestionarFSM();
  tareaBoton(tiempoActual);
  tareaAlarma(tiempoActual);
}