 //-----Importation des bibliotheques utilisées----//
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

//----créationtion d'une variable pour un timer----//
unsigned long lastTime;

RF24 radio(9, 10);   // assignation des pin pour le module radio
const byte address[6] = "00001"; // adresse de liason entre les 2
const char start[] = "Waking Up ..."; // premier message a envoyer pour démarage

MPU6050 mpu6050(Wire); // inistalisation du gyroscope en I2C

const ledR = 5; //pin led
const ledG = 2; //pin led
const ledB = 3; //pin led
const butPlus = 4; //pin bouton plus
char plusState = 0; //état bouton plus
const butMoins = 7;  //pin bouton moins
char moinsState = 0;  //état bouton moins
const x_key = A0; // axe X joystick
const y_key = A1; //axe Y joystick
int SW = A3;  // Switch pour changer de mode
//----Variable pour les axes X-Y ----//
int x_val_joy = 0;  
int y_val_joy = 0;
int x_val_gyr = 0;
int y_val_gyr = 0;
float map_x_val_joy = 0;
float map_y_val_joy = 0;
float map_x_val_gyr = 0;
float map_y_val_gyr = 0;
//---- Variable pour le trim ----//
float trimGyr = 0;


int values[2] = {0,90}; // {AXE X, AXE Y} // Array contenant les valeurs
char mode = 0;  // Mode entre gyro et joystick

void setup() {
  Serial.begin(9600);
  pinMode(ledR, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(x_key, INPUT);
  pinMode(y_key, INPUT);
  pinMode(butMoins, INPUT);
  pinMode(butPlus, INPUT);
  pinMode(SW, INPUT);
  digitalWrite(ledR, HIGH);
  //----Initalisation de la radio----//
  radio.begin();
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.setPALevel(RF24_PA_MIN);
  //----Initalisation du gyroscope----//
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  /* Boucle avec clignotement rapide la led pour créé la communication
     entre les 2 modules radio*/
  int i = 0;
  while (!radio.write(&start, sizeof(start))){
    if (i == 0){
      Serial.println("Les 2 appareils ne communiquent pas encore");
    }else if(odd(i)){
      analogWrite(ledR,255);
    }else{
      digitalWrite(ledR, LOW);
    }
    i++;
    delay(100);
  }
    /*Boucle avec clignotement lent la led laissant le temps au moteur de se démarré*/
  for(int i = 10; i>0; i--){
   if(odd(i) == false){
    analogWrite(ledR, 255);
   }
   else{
    analogWrite(ledR, 0);
   }
    delay(500);
  }
}

void loop() {
  /* Lecture des valeurs toutes les 0.15s*/
  if (millis() > lastTime + 150){
    plusState = digitalRead(butPlus);
    moinsState = digitalRead(butMoins);
    x_val_joy = analogRead(x_key);
    y_val_joy = analogRead(y_key);
    mpu6050.update();
    x_val_gyr = mpu6050.getAngleX();
    y_val_gyr = mpu6050.getAngleY();
     
    lastTime = millis();
  }
  /* Controle du trim */
  if (plusState == LOW){
    trimGyr = (trimGyr + 0.3);
  }
  if (moinsState == LOW){
    trimGyr = (trimGyr - 0.3);
  }
  /* Choix du mode */
  if (analogRead(SW) < 512){
      mode = 0;
    }
    else{
      mode = 1;
    }

  switch(mode){
    case 0:  // MODE JOYSTICK
      map_x_val_joy =  map(x_val_joy, 276, 767, 1300, 1600); // encardement de la valeur X du joystick 
      map_y_val_joy =  map(y_val_joy, 0, 1023, 50, 120); // encardement de la valeur Y du joystick 
      /* Assignation des valeures dans la liste qui va etre envoyée*/
      values[0] = map_x_val_joy;
      values[1] = map_y_val_joy;
      break;

    case 1:  // MODE ACCEL
      map_x_val_gyr = constrain(map(((x_val_gyr)*5),-255,255,1000,2000), 1300, 1600); // encardement de la valeur X du gyroscope
      map_y_val_gyr = constrain(((map(y_val_gyr, -50, 50, 50, 120))+trimGyr),50,120); // encardement de la valeur Y du gyroscope
      /* Assignation des valeures dans la liste qui va etre envoyée*/
      values[0] = map_x_val_gyr;
      values[1] = map_y_val_gyr;
      break;
  }

  /* envoi de l'array contenant les valeurs toutes les 0.1s*/
  if (millis()> lastTime + 100){
    bool tx_values = radio.write(&values, sizeof(values)); 
    //Led en rouge si la liste ne s'envoi pas et et en vert si elle s'envoit
    if (!tx_values){
      digitalWrite(ledR, HIGH);
      digitalWrite(ledG, LOW);
     }else{
      digitalWrite(ledG, HIGH);
      digitalWrite(ledR, LOW);
     }
    }
}

/* Fonction renvoyant vrai si le chiffre est pair*/
int odd(char x){
  if((x % 2)==0){
    return true;
  }
  else{
    return false;
  }
}
