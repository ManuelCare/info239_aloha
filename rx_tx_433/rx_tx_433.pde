/*
ver ::cl 20120520
Configuracion basica para modulo receptor  RR 10
Utiliza libreria VirtualWire.h
pin 01 5v
pin 02 Tierra
pin 03 antena externa
pin 07 tierra
pin 10 5v
pin 11 tierra
pin 12 5v
pin 14 Arduino pin digital 2
pin 15 5v
++++++++++++++++++++++++++++++++++++++++++++++
Configuracion basica para modulo transmisor RT 11
Utiliza libreria VirtualWire.h
pin 01 entrada desde Arduino pin digital 3
pin 02 Tierra
pin 07 tierra
pin 08 antena externa
pin 09 tierra
pin 10 5v
*/

#include <VirtualWire.h>

uint8_t mensaje[8] = "GRUPO_08";
uint8_t idx;
uint8_t origen = 8; //(int del grupo);
uint8_t destino = 0;//(int del grupo destino);
uint8_t letra;// = (la letra del mensaje que se enviara en el paquete)
uint8_t paquete[3]; //{origen, destino, letra}
uint8_t buf[3];
uint8_t BUFFER_SIZE = 24;
char mensajes[16][8];
uint8_t mensajes_filled[16];
int wait_time;
bool pkg_read;
uint8_t o,d,l; //origen destino y letra del paquete recibido

//variables de analisis de datos
unsigned long previousMillis = 0;
const unsigned long interval = 5000;
char formattedText[20];
uint16_t loops = 0;
uint16_t colisiones[16];

void setup(){
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_set_tx_pin(3);
    vw_rx_start();
    //inicializacion de variables
    paquete[0] = origen;
    paquete[1] = destino;
    wait_time = 0;
}
bool readPackage() {
  if(!vw_get_message(buf, BUFFER_SIZE)) return false;
  o = buf[0];
  d = buf[1];
  l = buf[2];
  return true;
}
bool collision(){
  return( (destino==d) && (origen!=o));
}
void wait(){
  wait_time += random(0,501);
  delay(wait_time);
}
void printStatus(){
  unsigned long currentMillis = millis();
  //Serial.print(currentMillis);Serial.print(" "); Serial.println(currentMillis - previousMillis);
  if (currentMillis - previousMillis >= interval) {
    Serial.println("Grupos | mensaje  | colisiones");
    for(int i=1; i<16; i++){
      sprintf(formattedText, "%-9d", i);
      Serial.print(formattedText);
      for(int j=0; j<8; j++)
        Serial.print(mensajes[i][j]);
      Serial.println("   " + String((int)colisiones[i]));
    }
    Serial.println("Loops: " + String((int)loops)+ "| Colisiones: " + String((int)colisiones[0]));
    previousMillis = currentMillis;
  }
  
}
void enviar(){
  paquete[2] = mensaje[idx];
  vw_send((uint8_t *)paquete, strlen(paquete));
  pkg_read = readPackage();
  if(pkg_read && collision()){
    //Serial.println("Colision con grupo " + String(buf[0]));
    colisiones[0]++;
    colisiones[buf[0]]++;
    digitalWrite(13, HIGH);
    wait();
  }
  else{
    //Serial.println("Enviada la letra " + String((char)paquete[2]));
    digitalWrite(13, LOW);
    idx = ((idx==7) ? 0 : idx+1);
    wait_time = 0;
  }
}
uint8_t molde[6] = "GRUPO_";
bool isFilled(int i){
  return ((mensajes_filled[o]>>i)%2) == 1;
}
void fill(int i){
  int t = 1;
  t = (t<<i);
  mensajes_filled[o] |= t;
}
int desired_pos(){
  uint8_t i = 0;
  while(i<6 && molde[i] != l) i++; //si es una letra de "GRUPO_"
  if(i<6) return i;
  if(o>9 ){ //si el grupo es mayor a 9, por lo tanto puede tener un 1 en la ultima o penultima pos
    if(l=='1'){ // entonces si recibimos un 1
      if(isFilled(6)) { // preguntamos si ya hay un 1 en la primera pos
        return 7; // para ponerlo en la ultima pos
      }
      else{ //sino lo ponemos en la primera pos
        return 6;
      }
    }
    else if(48<l && l<57){
      return 7;
    }
  }
  if(l=='0') return 6;
  if(48<l && l<57) return 7;
}
void recibir(){
  pkg_read = readPackage();
  if(pkg_read){
    if(d == origen && d == 0){
      uint8_t dp = desired_pos();
      if(dp!= -1){ // -1 marca caracteres que no deben ser puestos
        if(!isFilled(dp)){
          mensajes[o][dp] = l;
          fill(dp);

          if(mensajes_filled[o]==0b11111111){ // si estan todas las pos llenas
            Serial.print("Mensaje recibido del grupo "+ String((char)o)+ mensajes[o]);
            mensajes_filled[o] = 0;
          }
        }
      }
    }
  }
}
void loop(){
  enviar();
  recibir();
  printStatus();
  loops++;
  delay(random(100,200));
}