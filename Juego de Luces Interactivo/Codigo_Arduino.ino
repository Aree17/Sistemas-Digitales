const int LED_PINS[] = {2, 3, 4, 5, 6, 7};
const int BOTON_PIN = 8;
const int NUM_LEDS = 6;

int patronActual = 0;

int ultimoEstadoBoton = LOW;
int lecturaBoton = LOW;
unsigned long ultimoTiempoDebounce = 0;
const unsigned long debounceDelay = 50;

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }

  pinMode(BOTON_PIN, INPUT);
  randomSeed(analogRead(A0));
}

void loop() {
  leerBoton();

  switch (patronActual) {
    case 0:
      patronSecuencia();
      break;
    case 1:
      patronPersecucion();
      break;
    case 2:
      patronParpadeo();
      break;
    case 3:
      patronAleatorio();
      break;
    case 4:
      patronOnda();
      break;
  }
}

void leerBoton() {
  int lectura = digitalRead(BOTON_PIN);

  if (lectura != ultimoEstadoBoton) {
    ultimoTiempoDebounce = millis();
  }

  if ((millis() - ultimoTiempoDebounce) > debounceDelay) {
    if (lectura != lecturaBoton) {
      lecturaBoton = lectura;

      if (lecturaBoton == HIGH) {
        patronActual++;
        if (patronActual > 4) {
          patronActual = 0;
        }
      }
    }
  }

  ultimoEstadoBoton = lectura;
}

void apagarTodos() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void patronSecuencia() {
  for (int i = 0; i < NUM_LEDS; i++) {
    apagarTodos();
    digitalWrite(LED_PINS[i], HIGH);

    unsigned long inicio = millis();
    while (millis() - inicio < 150) {
      leerBoton();
      if (patronActual != 0) {
        apagarTodos();
        return;
      }
    }
  }
}

void patronPersecucion() {
  for (int i = 0; i < NUM_LEDS; i++) {
    apagarTodos();
    digitalWrite(LED_PINS[i], HIGH);

    unsigned long inicio = millis();
    while (millis() - inicio < 100) {
      leerBoton();
      if (patronActual != 1) {
        apagarTodos();
        return;
      }
    }
  }

  for (int i = NUM_LEDS - 2; i > 0; i--) {
    apagarTodos();
    digitalWrite(LED_PINS[i], HIGH);

    unsigned long inicio = millis();
    while (millis() - inicio < 100) {
      leerBoton();
      if (patronActual != 1) {
        apagarTodos();
        return;
      }
    }
  }
}

void patronParpadeo() {
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      digitalWrite(LED_PINS[i], HIGH);
    }

    unsigned long inicio1 = millis();
    while (millis() - inicio1 < 200) {
      leerBoton();
      if (patronActual != 2) {
        apagarTodos();
        return;
      }
    }

    for (int i = 0; i < NUM_LEDS; i++) {
      digitalWrite(LED_PINS[i], LOW);
    }

    unsigned long inicio2 = millis();
    while (millis() - inicio2 < 200) {
      leerBoton();
      if (patronActual != 2) {
        apagarTodos();
        return;
      }
    }
  }
}

void patronAleatorio() {
  apagarTodos();
  int led = random(0, NUM_LEDS);
  digitalWrite(LED_PINS[led], HIGH);

  unsigned long inicio = millis();
  while (millis() - inicio < 200) {
    leerBoton();
    if (patronActual != 3) {
      apagarTodos();
      return;
    }
  }

  digitalWrite(LED_PINS[led], LOW);
}

void patronOnda() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], HIGH);

    unsigned long inicio = millis();
    while (millis() - inicio < 120) {
      leerBoton();
      if (patronActual != 4) {
        apagarTodos();
        return;
      }
    }
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);

    unsigned long inicio = millis();
    while (millis() - inicio < 120) {
      leerBoton();
      if (patronActual != 4) {
        apagarTodos();
        return;
      }
    }
  }
}