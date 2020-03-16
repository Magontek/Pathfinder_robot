#include <printf.h>

#include <Servo.h>
#include <NewPing.h>


#include "motor_driver.h"
#include "ultramaper.h"
#include "tracker_router.h"
#include "rxtx.h"


//encoders
const uint8 Pin_encoder1 = PB4;
const uint8 Pin_encoder2 = PB5;
int cont_encoder_1, cont_encoder_2;
int entrada_m1,entrada_m2;

//Timer
HardwareTimer timer(4);
HardwareTimer pwm1(2);
HardwareTimer pwm2(3);

//Led
const uint8 ledPIN = PC13;

//boton
const uint8 BotonPIN = PB12;

//servo------------------------------------
const uint8 ServoPIN = PB1;
Servo Servo1;

//sonar-------------------------------------
const uint8 TRIGGER_PIN = PB11;
const uint8 ECHO_PIN = PB10;
const uint8 MAX_DISTANCE = 50;
NewPing Sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Ultramaper umap;

//Motores------------------------------------------
const uint8 motor2_a = PA0; //Derecho Adelante
const uint8 motor2_b = PA1; //Derecho Atras
const uint8 motor1_a = PA3; //Izquierdo Adelante
const uint8 motor1_b = PA2; //Izquierdo Atras

//proceso de datos
tracker_router track(78, entrada_m1, entrada_m2);

#define KP 200
#define KI 50
#define KD 100
#define OUTPUT_MIN 0
#define OUTPUT_MAX 52535
#define OUTPUT_OFFSET 8000
motor_driver motores(motor1_a,motor1_b, motor2_a, motor2_b, entrada_m1,entrada_m2);

//RF----------------------------------
//Declaremos los pines CE y el CSN Stm32
static int CE_PIN=PB0;
static int CSN_PIN=PA4;

//Variable con la dirección del canal por donde se va a transmitir
byte direccion[6] ={"1node"};

//creamos el objeto radio (NRF24L01)
RF24 radio(CE_PIN, CSN_PIN);

//Variables
char text[32]="";

void int_enc_1(){
  cont_encoder_1++;
}

void int_enc_2(){
  cont_encoder_2++;
}

void led_blink(byte times){
  for (byte i=0;i<times;i++){
    digitalWrite(ledPIN, LOW);
    delay(500);
    digitalWrite(ledPIN, HIGH);
    delay(500);
  }
}

void serial_catcher(){
  if (Serial.available() > 0){
    char Modo=Serial.read();
    int opt=Serial.parseInt();
    int opt1=Serial.parseInt();
    int opt2=Serial.parseInt();
    switch (Modo){
      case 'A':
        motores.derecho(opt);
        Serial.println("Avanza");
      break;
      case 'D':
        Serial.println("Se detiene");
        motores.derecho(0);
      break;
      case 'P':
      if(Serial.isConnected() && (Serial.getDTR() || Serial.getRTS())) {
           motores.print_data(opt);
      }
      break;
      case 'Q':
        motores.set_pid(opt,opt1,opt2, OUTPUT_MIN, OUTPUT_MAX, OUTPUT_OFFSET);
      break;
      case 'C':

      break;
      default:
        Serial.print("Comando invalido");
    }
    Serial.flush();
    Serial.println("S");
  }
}

bool automatico=0;

void rf_catcher(){
  if (radio.available()){             //Looking for the data.
    while (radio.available()) {       // While there is data ready
          radio.read( &text, 32 );    // Get the payload
          switch (text[0]){
            case '1':
              Serial.println(text);
            break;
            case 'a': //test
              Serial.println(text);
            break;
            case 'b': //set directo de velocidad
              motores.set_velocidad(text[1], 0);
              motores.set_velocidad(text[2], 1);
            break;
            case 'c':
              automatico=!automatico;
            break;
          }
    }
    
  }
}

int milis=0;
int act_pid=0;
//Operativo
void handler_timer(){

 
  //Velocidades
  act_pid++;
  if(act_pid==200){
    entrada_m1=cont_encoder_1;
    entrada_m2=cont_encoder_2;
    cont_encoder_1=0;
    cont_encoder_2=0;
    act_pid=0;

    motores.actualizar();
  }

  //segundos
  milis++;
  if (milis==1000){
    milis=0;
  }

}

void setup()
{  
  //Sonar---------------------------------
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(ledPIN, OUTPUT);
  pinMode(BotonPIN, INPUT);
  pinMode(ServoPIN, OUTPUT);

  //motores---------------------------------------------
  //encoder
  pinMode(Pin_encoder1, INPUT_PULLUP);
  pinMode(Pin_encoder2, INPUT_PULLUP);
  attachInterrupt(Pin_encoder1, int_enc_1, CHANGE);
  attachInterrupt(Pin_encoder2, int_enc_2, CHANGE);

  //IO motores
  pinMode(motor1_a, PWM);
  pinMode(motor1_b, PWM);
  pinMode(motor2_a, PWM);
  pinMode(motor2_b, PWM);
  pwm1.setPrescaleFactor(21);
  motores.set_pid(KP,KI,KD, OUTPUT_MIN, OUTPUT_MAX, OUTPUT_OFFSET);

  //traza
  track.set_max_min_real(75, 25);

  led_blink(1);
  //Puerto serie----------------------------
  Serial.begin(115200);
  //Serial1.begin(115200);

  //Scanner y servo-------------------------------------
  Servo1.attach(ServoPIN);
  umap.atachSonar(Sonar);
  umap.atachServo(Servo1);
  Servo1.write(0);
  
  //inicializamos el NRF24L01----------------------------
  radio.begin();
  delay(100);

  pinMode(PB4, INPUT_PULLDOWN);
  
  if(radio.isChipConnected()){
    Serial.print("Chip connected");
    digitalWrite(ledPIN, HIGH);
  }
  
  //Abrimos un canal de escritura
  radio.openReadingPipe(1, direccion);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver
  
  

  timer.pause();
  // Set up period
  timer.setPeriod(1000); // in microseconds
  // Set up an interrupt on channel 1
  timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  timer.setCompare(TIMER_CH1, 1000);  // Interrupt 1 count after each update
  timer.attachCompare1Interrupt(handler_timer);
  // Refresh the timer's count, prescale, and overflow
  timer.refresh();
  // Start the timer counting
  timer.resume();
}

int estado=0;

void rutina(){
  switch (estado){
    case 0:
      umap.mapeo();
      
    break;
    case 1:
      
    break;
  }
  
  
}

void loop()
{
  //led_blink(1);
  //Serial.println("Loop");
  serial_catcher();
  rf_catcher();
  if(automatico){
    rutina();
  }
  //Serial1.print("Serial 1 ");
  //umap.scanNext();
  //delay(100);
}
