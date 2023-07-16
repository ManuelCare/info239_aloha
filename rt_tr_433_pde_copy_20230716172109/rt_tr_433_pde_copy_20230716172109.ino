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
uint8_t mensaje[8] = "GRUPO_08"; uint8_t origen = 8; uint8_t destino = 0; uint8_t letra; uint8_t paquete[3];
uint8_t collisions_in_a_row = 0;
uint8_t idx = 0;
uint8_t m_template[6] = "GRUPO_";
bool hayMensaje;
uint8_t emisor, receptor, contenido;

unsigned long previousMillis = 0;
unsigned long timeElapsed;
unsigned long timeTillNextSend;
void setup() {
  
  Serial.begin(9600);
    pinMode(13, OUTPUT);
    vw_set_ptt_inverted(true); 
    vw_setup(2000);
    vw_set_rx_pin(2);
    vw_set_tx_pin(3);
    vw_rx_start();
    //
    paquete[0] = origen; paquete[1] = destino; paquete[2] = mensaje[idx];
}

bool leer(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if(vw_get_message(buf, &buflen)){
    emisor = buf[0]; receptor = buf[1]; contenido = buf[2];
    return true;
  }
  return false;
}

void enviar(){vw_send(paquete, 3);}
void manejarEnvio(){
  if(timeElapsed > timeTillNextSend) timeTillNextSend = 0;
  else timeTillNextSend -= timeElapsed;
  if(hayMensaje && (destino==receptor) && (origen!=emisor)){
    collisions_in_a_row++;
    timeTillNextSend = collisions_in_a_row*random(0 ,501);
    if(collisions_in_a_row>=10) timeTillNextSend = 0;
  }
  else{
    idx = ((idx==7) ? 0 : idx+1);
    paquete[2] = mensaje[idx];
    collisions_in_a_row = 0;
  }
}
void recibir(){
  if(hayMensaje){
    Serial.println(String(emisor)+ " " + String(receptor)+ " " + String((char)contenido));
  }
}
void loop() {
  unsigned long currentMillis = millis();
  timeElapsed = currentMillis - previousMillis;
  if(timeTillNextSend==0) enviar();
  Serial.println(timeTillNextSend);
  hayMensaje = leer();
  manejarEnvio();
  recibir();
  previousMillis = currentMillis;
  delay(100);
}
