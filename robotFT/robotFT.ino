/*
  Programa para manejar el robot Tatu-bot de Futhur Tech
 */

#include "tatubot.h"


Robot tatubot;


void setup(){
  tatubot.iniciar();
  tatubot.despertar();
}

void loop(){
  if (digitalRead(PIN_INICIO_ORDENES) == LOW) {
    Serial.print("Escuchando ordenes \n");
    tatubot.dibujarCaritaEntusiasmada();
    tatubot.reproducirSonido(2, 1900);         
    tatubot.escucharOrdenes();
  }
  if (digitalRead(PIN_ORDEN_AVANZAR) == LOW) {          
    tatubot.dibujarCaritaFeliz();
    tatubot.reproducirSonido(5, 20000);
  }
  if (digitalRead(PIN_ORDEN_REVERSA) == LOW) {          
    tatubot.dibujarCaritaEntusiasmada();
    tatubot.reproducirSonido(5, 20000); 
  }
  if (digitalRead(PIN_ORDEN_GIRO_DERECHA) == LOW) {          
    tatubot.dibujarCaritaEnojada();
    tatubot.reproducirSonido(5, 20000);
  }
  if (digitalRead(PIN_ORDEN_GIRO_IZQUIERDA) == LOW) {          
    /*tatubot.dibujarCaritaSorprendida();
    tatubot.reproducirSonido(5, 20000);*/
    tatubot.avisarBajaBateria();
  }
  if (digitalRead(PIN_FIN_ORDENES) == LOW) {          
    tatubot.dibujarCaritaTriste();
    tatubot.reproducirSonido(5, 750); 
  }
  if (digitalRead(PIN_RESET) == LOW) {          
    /*tatubot.dibujarCaritaDormido();
    tatubot.reproducirSonido(7, 2000); */
    tatubot.avisarMediaCapacidad();
  }
  tatubot.calcularCasosDeVelocidad();
  tatubot.revisarNivelDeBateria();
  
}
