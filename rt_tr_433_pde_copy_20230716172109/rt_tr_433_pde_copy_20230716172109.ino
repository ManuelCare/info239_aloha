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
char mensajes[16][8];
uint8_t mensajes_filled[16];
uint16_t colisiones[16];
uint16_t envios[16];

unsigned long previousMillis = 0;
unsigned long timeElapsed;
unsigned long timeTillNextSend;
unsigned long timeTillNextPrintStatus = 5000;
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
  if(timeTillNextSend==0){
    if(hayMensaje && (destino==receptor) && (origen!=emisor)){
      collisions_in_a_row++;
      colisiones[0]++;
      colisiones[emisor]++;
      timeTillNextSend = 500 + collisions_in_a_row*random(0 ,501);
      if(collisions_in_a_row>=10) timeTillNextSend = 0;
    }
    else{
      envios[0]++;
      envios[emisor]++;
      timeTillNextSend=500;
      idx = ((idx==7) ? 0 : idx+1);
      paquete[2] = mensaje[idx];
      collisions_in_a_row = 0;
    }
  }
}
//////////////
bool isFilled(int i){
  return ((mensajes_filled[emisor]>>i)%2) == 1;
}
void fill(int i){
  int t = 1;
  t = (t<<i);
  mensajes_filled[emisor] |= t;
}
int desired_pos(){
  uint8_t i = 0;
  while(i<6 && m_template[i] != contenido) i++; //si es una letra de "GRUPO_"
  if(i<6) return i;
  if(emisor>9 ){ //si el grupo es mayor a 9, por lo tanto puede tener un 1 en la ultima o penultima pos
    if(contenido=='1'){ // entonces si recibimos un 1
      if(isFilled(6)) { // preguntamos si ya hay un 1 en la primera pos
        return 7; // para ponerlo en la ultima pos
      }
      else{ //sino lo ponemos en la primera pos
        return 6;
      }
    }
    else if(48<contenido && contenido<57){
      return 7;
    }
  }
  if(contenido=='0') return 6;
  if(48<contenido && contenido<57) return 7;
  return -1;
}
void recibir(){
  if(hayMensaje){
    Serial.println(String(emisor)+ " " + String(receptor)+ " " + String((char)contenido));
  }

  if(receptor == origen || receptor == 0){
      uint8_t dp = desired_pos();
      if(dp!= -1){ // -1 marca caracteres que no deben ser puestos
        if(!isFilled(dp)){
          mensajes[emisor][dp] = contenido;
          fill(dp);

          if(mensajes_filled[emisor]==0b11111111){ // si estan todas las pos llenas
            Serial.print("Mensaje recibido del grupo "+ String((char)emisor)+ mensajes[emisor]);
            mensajes_filled[emisor] = 0;
          }
        }
      }
    }
  
}
char formattedText[20];
void printStatus(){
  if(timeElapsed > timeTillNextPrintStatus) timeTillNextPrintStatus = 0;
  else timeTillNextPrintStatus -= timeElapsed;
  //Serial.print(currentMillis);Serial.print(" "); Serial.println(currentMillis - previousMillis);
  if (timeTillNextPrintStatus == 0) {
    Serial.println("Grupos | mensaje  | colisiones | Envios");
    for(int i=0; i<16; i++){
      sprintf(formattedText, "%-9d", i);
      Serial.print(formattedText);
      for(int j=0; j<8; j++)
      Serial.print(mensajes[i][j]);
      sprintf(formattedText, "      %-9d", colisiones[i]);
      Serial.print(formattedText);
      Serial.println(envios[i]);
    }
    Serial.println("Envios: " + String((int)envios[0])+ "| Colisiones: " + String((int)colisiones[0]));
    timeTillNextPrintStatus = 5000;
  }
}
void loop() {
  unsigned long currentMillis = millis();
  timeElapsed = currentMillis - previousMillis;
  if(timeTillNextSend==0) enviar();
  hayMensaje = leer();
  manejarEnvio();
  recibir();
  //printStatus();
  previousMillis = currentMillis;
  delay(100);
}
