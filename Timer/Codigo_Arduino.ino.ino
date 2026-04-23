// Pines de entrada
const int pin555 = 2;
const int pinFF  = 3;

// Canal 1: señal del 555
int estadoActual1 = LOW;
int estadoAnterior1 = LOW;

unsigned long tiempoAnterior1 = 0;
unsigned long periodo1 = 0;

unsigned long tiempoCambio1 = 0;
unsigned long tHigh1 = 0;
unsigned long tLow1 = 0;

// Canal 2: señal del Flip-Flop
int estadoActual2 = LOW;
int estadoAnterior2 = LOW;

unsigned long tiempoAnterior2 = 0;
unsigned long periodo2 = 0;

unsigned long tiempoCambio2 = 0;
unsigned long tHigh2 = 0;
unsigned long tLow2 = 0;

// Control de tiempo
unsigned long inicioFase = 0;
unsigned long ultimoPrint = 0;
unsigned long ultimoPlot = 0;

// Duración de cada fase
const unsigned long duracionFase = 2000;
const unsigned long intervaloPlot = 10;
const unsigned long intervaloTexto = 1000;

// true = gráfica, false = texto
bool faseGrafica = true;

void setup() {
  pinMode(pin555, INPUT);
  pinMode(pinFF, INPUT);

  Serial.begin(9600);
  inicioFase = millis();
}

void loop() {
  unsigned long tiempoActual = millis();

  // Cambia entre fase gráfica y fase de texto cada 2 segundos
  if (tiempoActual - inicioFase >= duracionFase) {
    inicioFase = tiempoActual;
    faseGrafica = !faseGrafica;
  }

  // Lectura del canal 1
  estadoActual1 = digitalRead(pin555);

  // Detecta flanco ascendente para calcular periodo
  if (estadoActual1 == HIGH && estadoAnterior1 == LOW) {
    periodo1 = tiempoActual - tiempoAnterior1;
    tiempoAnterior1 = tiempoActual;
  }

  // Mide tiempos en HIGH y LOW
  if (estadoActual1 != estadoAnterior1) {
    if (estadoActual1 == HIGH) {
      tLow1 = tiempoActual - tiempoCambio1;
    } else {
      tHigh1 = tiempoActual - tiempoCambio1;
    }
    tiempoCambio1 = tiempoActual;
  }

  estadoAnterior1 = estadoActual1;

  // Lectura del canal 2
  estadoActual2 = digitalRead(pinFF);

  // Detecta flanco ascendente para calcular periodo
  if (estadoActual2 == HIGH && estadoAnterior2 == LOW) {
    periodo2 = tiempoActual - tiempoAnterior2;
    tiempoAnterior2 = tiempoActual;
  }

  // Mide tiempos en HIGH y LOW
  if (estadoActual2 != estadoAnterior2) {
    if (estadoActual2 == HIGH) {
      tLow2 = tiempoActual - tiempoCambio2;
    } else {
      tHigh2 = tiempoActual - tiempoCambio2;
    }
    tiempoCambio2 = tiempoActual;
  }

  estadoAnterior2 = estadoActual2;

  // Cálculo de frecuencia y duty cycle
  float freq1 = 0;
  float duty1 = 0;
  float freq2 = 0;
  float duty2 = 0;

  if (periodo1 > 0) {
    freq1 = 1000.0 / periodo1;
  }

  if ((tHigh1 + tLow1) > 0) {
    duty1 = (float)tHigh1 / (tHigh1 + tLow1) * 100.0;
  }

  if (periodo2 > 0) {
    freq2 = 1000.0 / periodo2;
  }

  if ((tHigh2 + tLow2) > 0) {
    duty2 = (float)tHigh2 / (tHigh2 + tLow2) * 100.0;
  }

  // Fase de gráfica: envía solo valores numéricos al Serial Plotter
  if (faseGrafica) {
    if (tiempoActual - ultimoPlot >= intervaloPlot) {
      ultimoPlot = tiempoActual;

      Serial.print(estadoActual1);
      Serial.print(",");
      Serial.println(estadoActual2);
    }
  }

  // Fase de texto: muestra resultados en el monitor serial
  else {
    if (tiempoActual - ultimoPrint >= intervaloTexto) {
      ultimoPrint = tiempoActual;

      Serial.println("====== SENAL 555 ======");
      Serial.print("Periodo (ms): "); Serial.println(periodo1);
      Serial.print("Frecuencia (Hz): "); Serial.println(freq1);
      Serial.print("Duty (%): "); Serial.println(duty1);

      Serial.println();

      Serial.println("====== FLIP-FLOP ======");
      Serial.print("Periodo (ms): "); Serial.println(periodo2);
      Serial.print("Frecuencia (Hz): "); Serial.println(freq2);
      Serial.print("Duty (%): "); Serial.println(duty2);

      Serial.println("\n------------------------\n");
    }
  }
}