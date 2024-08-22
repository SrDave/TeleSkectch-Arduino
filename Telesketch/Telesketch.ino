#include <MD_MAX72xx.h>
#include <SPI.h>

// Definir los pines del CLK, DIN y CS para la pantalla MD_MAX72xx y los botones 1 y 2
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10
#define BOTON1 2
#define BOTON2 3
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// Crear un objeto de pantalla MD_MAX72xx con los pines definidos, tipo de matriz y número de matrices
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 4);
int MAX_COL = 31;
int MAX_ROW = 7;

// Definir la posición del punto que parpadea al inicio
int x = 0;
int y = 0;

// Definir el temporizador para controlar el parpadeo
unsigned long previousMillis = 0;
unsigned long previous_Millis = 0;
const long interval = 500;  // La mitad de un segundo

// Variables para el control de la palanca de mando
int DEAD_ZONE = 100;
int ZONE_ONE = DEAD_ZONE * 2;
int ZONE_TWO = DEAD_ZONE * 3;
int ZONE_THREE = DEAD_ZONE * 4;
int ZONE_FOUR = DEAD_ZONE * 5;

int mode = 0;  //Para que inicie en modo = 0

//Variables del Joystick y estados de los botones, previos y actuales
int joystickButtonState = HIGH;
int xVal;
int yVal;
int buttonVal = 0;
const int JOYSTICK_X_PIN = A0;
const int JOYSTICK_Y_PIN = A1;
const int JOYSTICK_BUTTON_PIN = 4;
const int LED = 5;  //Pin del LED
int buttonState;
int boton1State = HIGH;
int boton2State = HIGH;
bool ledState = LOW;
bool led_State = LOW;
int prevX = -1;
int prevY = -1;
bool prevState = LOW;
bool prev_State = HIGH;
int prevJoyX = -1;
int prevJoyY = -1;
int statePrev1 = HIGH;
int statePrev2 = HIGH;
int prev_zx = 0;
int prev_zy = 0;

//Definimos la función para mover posiciones del LED seguún grado del joystick.
int Zonas(int zona) {
  if ((zona > 512 + DEAD_ZONE) && (zona < 512 + ZONE_TWO)) {
    return 1;
  } else if ((zona > 512 + ZONE_ONE) && (zona < 512 + ZONE_THREE)) {
    return 2;
  } else if ((zona > 512 + ZONE_TWO) && (zona < 512 + ZONE_FOUR)) {
    return 3;
  } else if ((zona > 512 + ZONE_FOUR)) {
    return 4;
  } else if ((zona < 512 - DEAD_ZONE) && (zona > 512 - ZONE_TWO)) {
    return 1;
  } else if ((zona < 512 - ZONE_ONE) && (zona > 512 - ZONE_THREE)) {
    return 2;
  } else if ((zona < 512 - ZONE_TWO) && (zona > 512 - ZONE_FOUR)) {
    return 3;
  } else if ((zona < 512 - ZONE_FOUR)) {
    return 4;
  } else {
    return 0;
  }
}

//Esta función lee la posición del joystick y devuelve valores x , y con la funcion Zonas.
void LecturaJoystick() {
  //Leemos los estados del joystick en x e y
  xVal = analogRead(JOYSTICK_X_PIN);
  yVal = analogRead(JOYSTICK_Y_PIN);
  //Detectamos el estado anterior para que no repita el movimiento.
  int zy = Zonas(yVal);
  int zx = Zonas(xVal);
  if (((zx != 0) || (zy != 0)) && (prev_zx == 0) && (prev_zy == 0)) {

    if (yVal > 514) {  // Mover hacia arriba
      y = y - Zonas(yVal);
    } else if (yVal < 510) {  // Mover hacia abajo
      y = y + Zonas(yVal);
    }

    if (xVal < 510) {  // Mover hacia la izquierda
      x = x - Zonas(xVal);
    } else if (xVal > 514) {  // Mover hacia la derecha
      x = x + Zonas(xVal);
    }

    //Definir los limites de la pantalla
    if (x >= MAX_ROW) {
      x = MAX_ROW;
    } else if (x < 0) {
      x = 0;
    }
    if (y >= MAX_COL) {
      y = MAX_COL;
    } else if (y < 0) {
      y = 0;
    }
  }
  prev_zx = zx;
  prev_zy = zy;
}

//Funcion para que parpadee el LED, podemos elegir PIN y duración
void LedParpadeo(int LED_PIN, int tiempo) {
  unsigned long current_Millis = millis();
  if (current_Millis - previous_Millis >= tiempo) {
    previous_Millis = current_Millis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  }
  digitalWrite(LED_PIN, ledState);
}

//Función que solo mueve el puntero del LED respetando el estado anterior, solo se mueve.
void ModoDesplazamiento() {

  unsigned long currentMillis = millis();            // obtener el tiempo actual
  if (currentMillis - previousMillis >= interval) {  // si ha pasado el intervalo de tiempo
    previousMillis = currentMillis;                  // actualizar el último tiempo registrado
    LecturaJoystick();
    if ((x != prevX) || (y != prevY)) {  //Si la posición actual de x , y no es igual que la anterior

      mx.setPoint(prevX, prevY, prevState);
      //Actualiza prevX y prevY
      prevX = x;
      prevY = y;
      prevState = mx.getPoint(x, y);  //Acumula el estado previo de encendido del LED
    }
    if (mx.getPoint(x, y)) {     // alterna el estado del LED
      mx.setPoint(x, y, false);  // enciende o apaga el LED en la posición de la matriz
    } else {
      mx.setPoint(x, y, true);
    }
  }
}

//Función que enciende los LEDs por donde pasa.
void ModoDibujo() {

  unsigned long currentMillis = millis();            // obtener el tiempo actual
  if (currentMillis - previousMillis >= interval) {  // si ha pasado el intervalo de tiempo
    previousMillis = currentMillis;                  // actualizar el último tiempo registrado
    LecturaJoystick();
    if ((x != prevX) || (y != prevY)) {  //Si la posición actual de x , y no es igual que la anterior

      dibujar_linea_bresenham(prevX, prevY, x, y, true);  //Usamos la funcion para dibujar todas las posiciones del LED
      //Actualiza prevX y prevY
      prevX = x;
      prevY = y;
    }
    if (mx.getPoint(x, y)) {     // alterna el estado del LED
      mx.setPoint(x, y, false);  // enciende o apaga el LED en la posición de la matriz
    } else {
      mx.setPoint(x, y, true);
    }
  }
}

//Función que borra (apaga) el LED por donde pasa
void ModoBorrado() {
  unsigned long currentMillis = millis();            // obtener el tiempo actual
  if (currentMillis - previousMillis >= interval) {  // si ha pasado el intervalo de tiempo
    previousMillis = currentMillis;                  // actualizar el último tiempo registrado
    LecturaJoystick();
    if ((x != prevX) || (y != prevY)) {                    //Si la posición actual de x , y no es igual que la anterior
      dibujar_linea_bresenham(prevX, prevY, x, y, false);  //Usamos la funcion para dibujar todas las posiciones del LED
      //Actualiza prevX y prevY
      prevX = x;
      prevY = y;
    }
    if (mx.getPoint(x, y)) {     // alterna el estado del LED
      mx.setPoint(x, y, false);  // enciende o apaga el LED en la posición de la matriz
    } else {
      mx.setPoint(x, y, true);
    }
  }
}


//Función que representa el algoritmo de Bresenham, une los puntos de principio y fin del ejeX y del ejeY, además un bool True False
void dibujar_linea_bresenham(int x0, int y0, int x1, int y1, bool TF) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (true) {
    mx.setPoint(x0, y0, TF);
    if (x0 == x1 && y0 == y1) {  //Si el primer x ó y y el último x ó y coinciden, break
      break;
    }
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

//Funciones que detectan el estado de los botones 1 y 2 y entran en un modo u otro según cual pulses.
void PulsarBoton1() {
  if (boton1State != statePrev1) {
    if (boton1State == LOW) {
      if (mode == 2) {  // Si viene de mode 2, cambia a mode 1.
        mode = 1;
      } else {  // Si viene de mode 0 cambia al 1, si viene de mode 1 cambia a 0.
        mode = 1 - mode;
      }
    }
    statePrev1 = boton1State;  // actualiza el estado.
  }
}

void PulsarBoton2() {
  if (boton2State != statePrev2) {
    if (boton2State == LOW) {
      if (mode == 1) {  // Si viene de mode 1, cambia a mode 2.
        mode = 2;
      } else {  // Si viene de mode 0 cambia al 2, si viene de mode 2 cambia a 0.
        mode = 2 - mode;
      }
    }
    statePrev2 = boton2State;  // actualiza el estado.
  }
}

void setup() {

  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  xVal = analogRead(JOYSTICK_X_PIN);
  yVal = analogRead(JOYSTICK_Y_PIN);
  buttonVal = digitalRead(JOYSTICK_BUTTON_PIN);


  // Inicializar la pantalla
  mx.begin();
  mx.clear();
}

void loop() {
  // Leer el estado del botón de la palanca de mando

  boton1State = digitalRead(BOTON1);
  boton2State = digitalRead(BOTON2);
  // Cambiar el modo al pulsar el botón de la pantalla
  PulsarBoton1();
  PulsarBoton2();

  if (mode == 0) {  // Modo de desplazamiento
    ModoDesplazamiento();
    buttonState = digitalRead(JOYSTICK_BUTTON_PIN);
    if (buttonState == LOW && prev_State == HIGH) {
      prevState = (prevState == LOW) ? HIGH : LOW;
    }
    digitalWrite(LED, LOW);
    prev_State = buttonState;
  }

  else if (mode == 1) {  // Modo de edición
    ModoDibujo();
    digitalWrite(LED, HIGH);  //Led encendido fijo
  } else if (mode == 2) {     // Modo Borrado
    ModoBorrado();
    LedParpadeo(LED, interval * 2);  //Led parapdeo 1 Hz
  }
}
