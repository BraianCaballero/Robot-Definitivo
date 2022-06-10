#include <ArduinoQueue.h>
#include "tatubot.h"
#include "LedControl.h"     
#include "caritas.h"
#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"


Motor motor0(IN1, IN2, ENA);
Motor motor1(IN4, IN3, ENB);
Led led1(PIN_R_A,PIN_G_A,PIN_B_A);
Led led2(PIN_R_B,PIN_G_B,PIN_B_B);
ArduinoQueue<int> colaOrdenes(CANTIDAD_ORDENES);
LedControl matrizLeds=LedControl(DIN,CLK,CS,MATRICES);
SoftwareSerial mySoftwareSerial(22, 23 ); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

bool avisoMediaCapacidad = false;
bool avisoBajaBateria = false;
bool contador0EnHigh = false;
bool contador1EnHigh = false;
bool banderaDeLineaNegra = true;
bool contemploSiLlegoALineaNegra = true;
int contador0;
int contador1;
float VELOCIDAD_MOTOR0 = 85;
float VELOCIDAD_MOTOR1 = 85; 
const int voltajeMax = 25000; // 25V -> 25000mV
int lecturaDigital; // 1023
float voltaje;
int volumenAudio = 27 ; //set value 0-30 
float leerTensionDeBateria() {
  lecturaDigital = analogRead(A7);
  voltaje = map(lecturaDigital, 0, 1023, 0, voltajeMax) / 1000.0;
  return voltaje;
  //voltaje = (lecturaDigital / 1023) * 5.0;
}
float promedioDeTension(){
  int acumuladorDelWhile = 0;
  float suma = 0;
  while(acumuladorDelWhile <= 20)
  {
    suma += leerTensionDeBateria();
    acumuladorDelWhile ++;
  }
  return suma/20;
}


void Robot::iniciar() {
  pinMode(pinEscucharOrdenes, INPUT_PULLUP);
  pinMode(pinAvanzar, INPUT_PULLUP);
  pinMode(pinReversa, INPUT_PULLUP);
  pinMode(pinGiroDerecha, INPUT_PULLUP);
  pinMode(pinGiroIzquierda, INPUT_PULLUP);
  pinMode(pinFinOrdenes, INPUT_PULLUP);
  pinMode(pinReset, INPUT_PULLUP);
  pinMode(SENSOR0, INPUT);
  pinMode(SENSOR1, INPUT);
  Serial.begin(115200);
  iniciarDFPlayerMini();
  motor0.iniciar();
  motor1.iniciar();
  led1.iniciar();
  led2.iniciar();
  matrizLeds.shutdown(0,false);     
  matrizLeds.setIntensity(0,4);    
  matrizLeds.clearDisplay(0);    
  matrizLeds.shutdown(1,false);    
  matrizLeds.setIntensity(1,4);    
  matrizLeds.clearDisplay(1); 
  Serial.print("La bateria esta en: ");
  Serial.println(promedioDeTension());
  Serial.print("La velocidad esta en :");
  Serial.println(VELOCIDAD_MOTOR0);
  delay(100);
}

void Robot::avisarMediaCapacidad(){
  reproducirSonido(9, 3000);
}

void Robot::avisarBajaBateria(){
  dibujarCarita(dormidoOjo, dormidoOjo);
  reproducirSonido(8, 4000);
}

void Robot::revisarNivelDeBateria(){
  float nivelActual = promedioDeTension();
  if (nivelActual <= 7.75 && nivelActual > 7.23 && avisoMediaCapacidad == false){
      avisarMediaCapacidad();
      avisoMediaCapacidad = true;
    }
  if (nivelActual <= 7.23 && avisoBajaBateria == false){
      avisarBajaBateria();
      avisoBajaBateria = true;
  }
}


void Robot::iniciarDFPlayerMini() {
  mySoftwareSerial.begin(9600 );
  myDFPlayer.begin(mySoftwareSerial);
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    //while (true);
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  //----Set volume----
  myDFPlayer.volume(volumenAudio);  //Set volume value (0~30).
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_BASS);
  //----Set device we use SD as default----
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
}

void Robot::reproducirSonido(int sonido, int tiempo) {
  myDFPlayer.play(sonido);
  delay(tiempo);
  myDFPlayer.pause();
}
 
void Robot::avanzar(int velocidad, int velocidad2) {
  contador0 = 0;
  contador1 = 0;
  contemploSiLlegoALineaNegra = true;
  banderaDeLineaNegra = true;
  movimientoAdelante(velocidad, velocidad2);
  //autocorregir();
  Serial.print("Tension= ");
  Serial.println(leerTensionDeBateria());
  
}

void Robot::retroceder(int velocidad, int velocidad2) {
  contador0 = 0;
  contador1 = 0;
  contemploSiLlegoALineaNegra = true;
  banderaDeLineaNegra = true;
  movimientoReversa(velocidad, velocidad2);
  Serial.print("Tension= ");
  Serial.println(leerTensionDeBateria());
  Serial.println("Realice el movimiento hacia atras");
}

void Robot::girarDerecha() {
  contador0 = 0;
  contador1 = 0;
  movimientoDerecha();
  Serial.print("Tension= ");
  Serial.println(leerTensionDeBateria());
}

void Robot::girarIzquierda() {
  contador0 = 0;
  contador1 = 0;
  movimientoIzquierda();
  Serial.print("Tension= ");
  Serial.println(leerTensionDeBateria());
}

void Robot::movimientoAdelante(int velocidad, int velocidad2)
{
  while(contemploSiLlegoALineaNegra)
  {
    motor0.adelante(velocidad);
    motor1.adelante(velocidad2);
    led1.rgb('g');
    led2.rgb('g');
    detectarLineaNegraEIgualar(velocidad);
    }
    while(contador0 <= 11 && contador1<= 11)
    {
      motor0.adelante(velocidad);
      motor1.adelante(velocidad2);
      led1.rgb('g');
      led2.rgb('g');
      contador();

    }
    pararRobot();
}

void Robot::movimientoReversa(int velocidad, int velocidad2)
{
  while(contemploSiLlegoALineaNegra)
  {
    motor0.atras(velocidad);
    motor1.atras(velocidad2);
    led1.rgb('r');
    led2.rgb('r');
    detectarLineaNegraEIgualarEnReversa(velocidad);
    }
  while(contador0 <= 5 && contador1<= 5) {
    motor0.atras(velocidad);
    motor1.atras(velocidad2);
    led1.rgb('g');
    led2.rgb('g');
    contador();
  }
    pararRobot();
}

void Robot::movimientoDerecha()
{
  //led 1 izq mirando de atras
  /*motor0.adelante(100);  
  motor1.adelante(100);  
  delay(125);            
  pararRobot(); */         
  while (contador1 < 18) {
      motor0.atras(100);
      motor1.adelante(100);
      delay(15);
      led1.rgb('b');
      contador();
  }
  pararRobot();
  led1.rgb('a');
}

void Robot::movimientoIzquierda()
{
  //led 2 derecha mirando de atras
 /* motor0.adelante(100);  
  motor1.adelante(100);  
  delay(125);            
  pararRobot();*/          
  while (contador0 < 16) {
      motor0.adelante(100);
      motor1.atras(100);
      delay(15);
      led2.rgb('b');
      contador();
    }
  pararRobot();
  led2.rgb('a');
}

void Robot::pararRobot() {
  motor0.parar();
  motor1.parar();
  led1.rgb('a');
  led2.rgb('a');
}

void Robot::autocorregir() {
 if (contador0 > contador1) {
  igualarRuedaAdelantada0();
 }
 else if (contador1 > contador0) {
  igualarRuedaAdelantada1();
 }
}

void Robot::igualarRuedaAdelantada0() 
{
  while (contador1 != contador0) {
    motor0.parar();
    motor1.adelante(75);
    contador();
  }
}

void Robot::igualarRuedaAdelantada1() 
{
  while (contador0 != contador1) {
    motor1.parar();
    motor0.adelante(75);
    contador();
  }
}

void Robot::autocorregirReversa() {
 if (contador0 > contador1) {
  igualarRuedaAdelantadaReversa0();
 }
 else if (contador1 > contador0) {
  igualarRuedaAdelantadaReversa1();
 }
}

void Robot::igualarRuedaAdelantadaReversa0() 
{
  while (contador0 != contador1) {
    motor0.parar();
    motor1.atras(85);
    contador();
  }
}

void Robot::igualarRuedaAdelantadaReversa1() 
{
  while (contador1 != contador0) {
    motor1.parar();
    motor0.atras(85);
    contador();
  }
}

void Robot::contador() {
  if (digitalRead(SENSOR1) == LOW && contador1EnHigh == false)  
  {
      contador1++; 
      contador1EnHigh = true;
  }
  if (digitalRead(SENSOR1) == HIGH)
  {
      contador1EnHigh = false;
  }
  if (digitalRead(SENSOR0) == LOW && contador0EnHigh == false )
  {
      contador0++;  
      contador0EnHigh = true;
  }
  if (digitalRead(SENSOR0) == HIGH)
  {
       contador0EnHigh = false;
  }
}

void Robot::detectarLineaNegraEIgualarEnReversa(int velocidad) {
  
if(banderaDeLineaNegra == true){
  if (digitalRead(SENSORDERECHO) == LOW && digitalRead(SENSORIZQUIERDO) == HIGH )
  {
    Serial.println("tengo que adelantar el motor derecho");
    moverMotor0HastaLineaNegra(velocidad);
    banderaDeLineaNegra = false;
    contemploSiLlegoALineaNegra = false;
  }
  if (digitalRead(SENSORDERECHO) == HIGH && digitalRead(SENSORIZQUIERDO) == LOW && banderaDeLineaNegra == true )
  {
    Serial.println("tengo que adelantar el motor izquierdo");
    moverMotor1HastaLineaNegra(velocidad);
    banderaDeLineaNegra = false;
    contemploSiLlegoALineaNegra = false;
  }
  
}
}

void Robot::moverMotor0HastaLineaNegra(int velocidad) {
  while (digitalRead(SENSORDERECHO)== LOW) {
    motor0.atras(velocidad);
    motor1.parar();
    delay(15);
    Serial.println("EStoy adelantando el motor derecho (0)");
  }
  //rueda parada retrocede al blanco
  while(digitalRead(SENSORIZQUIERDO)== LOW){
    motor1.adelante(velocidad);
    motor0.parar();
    }
  //rueda que se adelanto originalmente vuelve al blanco
    while(digitalRead(SENSORDERECHO)== HIGH){
    motor0.atras(velocidad);
    motor1.parar();
    }
  pararRobot();
  delay(500);
}

void Robot::moverMotor1HastaLineaNegra(int velocidad) {
  while (digitalRead(SENSORIZQUIERDO)== LOW){
    motor1.atras(velocidad);
    motor0.parar();
    delay(15);
    Serial.println("EStoy adelantando el motor izquierdo (1)");
  }
    //rueda parada retrocede al negro
  while(digitalRead(SENSORDERECHO)== LOW){
    motor0.adelante(velocidad);
    motor1.parar();
    }

   //rueda que se adelanto originalmente vuelve al blanco
    while(digitalRead(SENSORIZQUIERDO)== HIGH){
    motor0.parar();
    motor1.atras(velocidad);
    }
  pararRobot();
  delay(500);
}

void Robot::detectarLineaNegraEIgualar(int velocidad) {
  
if(banderaDeLineaNegra == true){
  if (digitalRead(SENSORDERECHO) == LOW && digitalRead(SENSORIZQUIERDO) == HIGH )
  {
    Serial.println("tengo que adelantar el motor derecho");
    adelantarMotorAtrasado0(velocidad);
    banderaDeLineaNegra = false;
    contemploSiLlegoALineaNegra = false;
  }
  if (digitalRead(SENSORDERECHO) == HIGH && digitalRead(SENSORIZQUIERDO) == LOW && banderaDeLineaNegra == true )
  {
    Serial.println("tengo que adelantar el motor izquierdo");
    adelantarMotorAtrasado1(velocidad);
    banderaDeLineaNegra = false;
    contemploSiLlegoALineaNegra = false;
  }
  
}
}

void Robot::adelantarMotorAtrasado0(int velocidad) {
  while (digitalRead(SENSORDERECHO)== LOW) {
    motor0.adelante(velocidad);
    motor1.parar();
    delay(15);
    Serial.println("EStoy adelantando el motor derecho (0)");
  }
  //rueda parada retrocede al blanco
  while(digitalRead(SENSORIZQUIERDO)== LOW){
    motor1.atras(velocidad);
    motor0.parar();
    }
  //rueda que se adelanto originalmente vuelve al blanco
    while(digitalRead(SENSORDERECHO)== HIGH){
    motor0.adelante(velocidad);
    motor1.parar();
    }
  pararRobot();
  delay(500);
}

void Robot::adelantarMotorAtrasado1(int velocidad) {
  while (digitalRead(SENSORIZQUIERDO)== LOW){
    motor1.adelante(velocidad);
    motor0.parar();
    delay(15);
    Serial.println("EStoy adelantando el motor izquierdo (1)");
  }
    //rueda parada retrocede al negro
  while(digitalRead(SENSORDERECHO)== LOW){
    motor0.atras(velocidad);
    motor1.parar();
    }

   //rueda que se adelanto originalmente vuelve al blanco
    while(digitalRead(SENSORIZQUIERDO)== HIGH){
    motor0.parar();
    motor1.adelante(velocidad);
    }
  pararRobot();
  delay(500);
}

void Robot::calcularCasosDeVelocidad(){
  /*
   Vmáx= 8.3
   Vmin= 6.2
   Vmáx - Vmin = DiferenciaDeVoltaje / 4
   0% = 6.2V
   25% = 6.725V
   50% = 7.25V
   75% = 7.775V
   100% = 8.3 o más...
  */
  float valorDeTension = promedioDeTension();
  
  /*if (6.2 < valorDeTension && valorDeTension < 6.725 ){
    VELOCIDAD_MOTOR0 = 150;
    VELOCIDAD_MOTOR1 = 150;
  }
  else if(6.725 <= valorDeTension  && valorDeTension  < 7.25 ){
    VELOCIDAD_MOTOR0 = 105;
    VELOCIDAD_MOTOR1 = 105;
  }
  else if(7.25 <= valorDeTension &&valorDeTension  <= 7.775 ){
    VELOCIDAD_MOTOR0 = 95;
    VELOCIDAD_MOTOR1 = 95;
  }
  else if (7.775 <= valorDeTension && valorDeTension  <= 8.5 ){
    VELOCIDAD_MOTOR0 = 80;
    VELOCIDAD_MOTOR1 = 80;
  }
  else {
    VELOCIDAD_MOTOR0 = 0;
    VELOCIDAD_MOTOR1 = 0;
  }*/
  
  //avisar al 10% que tiene sueño 7.23v audio 8
  //avisar que esta al 50% 7.75v audio 9
  //maximo de tension = 8.4 y minimo = 7.1
  if(7.10 <= valorDeTension  && valorDeTension  < 7.31 ){
    VELOCIDAD_MOTOR0 = 110;
    VELOCIDAD_MOTOR1 = 110;
  }
  else if(7.31 <= valorDeTension &&valorDeTension  <= 8.18 ){
    VELOCIDAD_MOTOR0 = 90;
    VELOCIDAD_MOTOR1 = 90;
  }
  else if (8.18 <= valorDeTension && valorDeTension  <= 8.5 ){
    VELOCIDAD_MOTOR0 = 80;
    VELOCIDAD_MOTOR1 = 80;
  }
  else {
    VELOCIDAD_MOTOR0 = 0;
    VELOCIDAD_MOTOR1 = 0;
  }
  //Serial.print("La velocidad actual es de:");
  //Serial.println(VELOCIDAD_MOTOR0);
}

void Robot::escucharOrdenes() {
  
  while(colaOrdenes.itemCount() <= CANTIDAD_ORDENES && digitalRead(pinFinOrdenes) != LOW) {
    
    if (digitalRead(pinAvanzar) == LOW) {
          dibujarCaritaSorprendida();          
          colaOrdenes.enqueue(1);
          led1.rgb('g');
          led2.rgb('g');
          Serial.print("Orden para avanzar en cola \n");
          reproducirSonido(5, 500); 
          //delay(500);
          led1.rgb('a');
          led2.rgb('a');
          dibujarCaritaFeliz();      
    }
    if (digitalRead(pinReversa) == LOW) {
          dibujarCaritaSorprendida();           
          colaOrdenes.enqueue(2);
          led1.rgb('r');
          led2.rgb('r');
          Serial.print("Orden para retroceder en cola \n");
          reproducirSonido(5, 500); 
          //delay(500);
          led1.rgb('a');
          led2.rgb('a');
          dibujarCaritaFeliz(); 
    }
    if (digitalRead(pinGiroDerecha) == LOW) {  
          dibujarCaritaSorprendida();         
          colaOrdenes.enqueue(3);
          led2.rgb('b');
          Serial.print("Orden para girar a la derecha en cola \n");
          reproducirSonido(5, 500); 
          //delay(500);
          led2.rgb('a');
          dibujarCaritaFeliz(); 
    }
    if (digitalRead(pinGiroIzquierda) == LOW) {    
          dibujarCaritaSorprendida();       
          colaOrdenes.enqueue(4);
          led1.rgb('b');
          Serial.print("Orden para girar a la izquierda en cola \n");
          reproducirSonido(5, 500); 
          //delay(500);
          led1.rgb('a');
          dibujarCaritaFeliz(); 
    }
   if (digitalRead (pinReset) == LOW) {
      while(colaOrdenes.itemCount() > 0)
      {
        colaOrdenes.dequeue();
      }
      Serial.print("resetee todas las ordenes");
      reproducirSonido(4, 1500);
      //delay(500);
  }
 } 
   reproducirSonido(3, 750);
  
  if(colaOrdenes.itemCount() < 3) {
        Serial.print("Se enoja por cancelar antes de las 3 ordenes");
        dibujarCaritaEnojada();
        delay(500);
  }
  Serial.print("Se ejecutan las ordenes \n");
  Serial.print(colaOrdenes.itemCount());
  ejecutarTodasLasOrdenes();
}

void Robot::ejecutarTodasLasOrdenes() {
  int cantidadOrdenes = colaOrdenes.itemCount();
  delay(1000);
  for(int i=0; i<cantidadOrdenes; i++) {
        Serial.print(colaOrdenes.itemCount());
        dibujarCaritaEntusiasmada();
        
        ejecutarOrden(colaOrdenes.dequeue());
        dibujarCaritaFeliz();
        delay(1000);
  }
  reproducirSonido(7, 2000); 
}


void Robot::ejecutarOrden(byte caso) {
  switch(caso) {
    case 1:
      avanzar(VELOCIDAD_MOTOR0,VELOCIDAD_MOTOR1);
      break;
    case 2:
      retroceder(VELOCIDAD_MOTOR0,VELOCIDAD_MOTOR1);
      break;
    case 3:
      girarDerecha();
      break;
    case 4:
      girarIzquierda();
      break;
  }
}
void Robot::dibujarCarita(byte ojos[8], byte boca[8]) {     
  for (int i = 0; i < 8; i++)     
  {
    matrizLeds.setRow(0,i,ojos[i]);   
  }
  for (int i = 0; i < 8; i++)     
  {
    matrizLeds.setRow(1,i,boca[i]);    
  }
}


void Robot::dibujarCaritaFeliz() { dibujarCarita(ojoFeliz,  ojoFeliz); }
void Robot::dibujarCaritaSorprendida() { dibujarCarita(ojoDefault,  ojoDefault); }
void Robot::dibujarCaritaEntusiasmada() { dibujarCarita(ojoCorazonCabeza,  ojoCorazonCabeza); }
void Robot::dibujarCaritaTriste() { dibujarCarita(ojoCerrado,  ojoCerrado); }
void Robot::dibujarCaritaEnojada() { dibujarCarita(ojoEnojadoDerCabeza,  ojoEnojadoIzqCabeza); }
void Robot::dibujarCaritaGuiniando() { dibujarCarita(ojoFeliz,  ojoCerrado); }
void Robot::dibujarCaritaDormido() {dibujarCarita(dormidoOjo, dormidoOjo); }
void Robot::despertar() {
  myDFPlayer.play(1);
  dibujarCarita(ojoCerrado,  ojoCerrado);
  delay(300);
  dibujarCarita(ojoDefault,  ojoDefault);
  delay(500);
  dibujarCarita(ojoCerrado,  ojoCerrado);
  delay(300);
  dibujarCarita(ojoFeliz,  ojoFeliz);
  delay(1000);
  dibujarCaritaGuiniando();
  delay(250);
  dibujarCaritaFeliz();
  delay(500);
  myDFPlayer.pause();
}

//---------------------------------------------
Motor::Motor(byte pinAtras, byte pinAdelante, byte pinVelocidad) {
  this->pinAtras = pinAtras;
  this->pinAdelante = pinAdelante;
  this->pinVelocidad = pinVelocidad;
}

void Motor::iniciar() {
  pinMode(pinAtras, OUTPUT);
  pinMode(pinAdelante, OUTPUT);
  pinMode(pinVelocidad, OUTPUT);
  this->parar();
}

void Motor::adelante(int velocidad) {
  analogWrite(pinVelocidad, velocidad);
  digitalWrite(pinAtras, LOW);
  digitalWrite(pinAdelante, HIGH);
}

void Motor::atras(int velocidad) {
  analogWrite(pinVelocidad, velocidad);
  digitalWrite(pinAdelante, LOW);
  digitalWrite(pinAtras, HIGH);
}

void Motor::parar() {
  analogWrite(pinVelocidad, 0);
}


//-------------------------------------------------------
Led::Led(byte ledR, byte ledG, byte ledB) {
  this->ledR = ledR;
  this->ledG = ledG;
  this->ledB = ledB;
}

void Led::iniciar() {
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  this->rgb('a');
}


void Led::rgb(char color) {
  switch (color) {
    case 'r': //red
      digitalWrite(ledR, LOW);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      break;
    case 'g': //green
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, LOW);
      digitalWrite(ledB, HIGH);
      break;
    case 'y': //yellow
      digitalWrite(ledR, LOW);
      digitalWrite(ledG, LOW);
      digitalWrite(ledB, HIGH);
      break;
    case 'b': //blue
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, LOW);
      break;
    case 'a': //apagado
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      break;
  }
}
