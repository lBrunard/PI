//-----Importation des bibliotheques utilisées----//
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//----créationtion d'une variable pour un timer----//
unsigned long lastTime;

//---- création des variables pour les composants----/
Servo servo1;
Servo moteur;

RF24 radio(9, 10); // assignation des pin pour le module radio
const byte address[6] = "00001"; // address // adresse de liason entre les 2 radios

int values[2] = {0,90};  // création de la liste pour les valeurs moteur et servo
bool rc = false;   // Création de la variable recieving boolean comme témoins de réception

void setup(){
    Serial.begin(9600);
    //----Initalisation de la radio----//
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.startListening();
    radio.setPALevel(RF24_PA_MIN);
    //----Initalistation des pins pour le moteur et servo----//
    servo1.attach(3);
    moteur.attach(5);
}

void loop(){
    if (radio.available()){ // La radion vérifie si elle recoit une valeur quelconque
      radio.read(&values, sizeof(values));  //décodage de la valeur recue 
      rc = true;  // le témoin de récpetion devient vrai
    }
    if (rc){  // Si le témoins de réception est vrai
      /*---- Simple vérification des valeurs recues et envoi des bonnes données au moteur et servo
              Si les valeurs ne sont pas bonnes, envoi de valeurs neutre au composants*/
        if ((values[1] < 50) || (values[1] > 120) || (values[0] < 1000) || (values[0] > 2000)){
           servo1.write(90);
           moteur.writeMicroseconds(1500);
        }
        else{
           servo1.write(values[1]);
           moteur.writeMicroseconds(values[0]);
        }
        lastTime = millis();
    }
    
    //---- Si la radio ne recois plus de signal depuis 0.2s, envoi de valeurs neutres au composants----//
    else if((!rc) && (millis() > lastTime + 200)){
        servo1.write(90);
        moteur.writeMicroseconds(1500);
    }
    rc = false;  //témoins de réception toujours faux en fin de boucle
}
