#include <DHT.h>                                //Library for dht sensor
#include <SoftwareSerial.h>                     //Library for serial communication between arduino and  ESp01

#define RX 2                                    //Defining Serial Receiver Pin of Arduino  
#define TX 3                                    //Defining Serial Transmitter Pin of Arduino
#define dht_apin 6                              // Analog Pin sensor is connected to
DHT dhtObject;                                  // Initialising an Object
String AP = "Honor";       // AP NAME
String PASS = "mohityadav"; // AP PASSWORD
String API = "YISB057VJ7GD4Y8G";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand;
boolean found = false;

int led = 8;              // variable for Red LED  (indicating if mask is removed)
int greenled = 4;         // variable for green LED(indicating close distance)
int buzzPin = 9;          // variable  for buzzer
int trigPin = 11;         // Variable for trigger pin of ULTRASONIC
int echoPin = 10;         // Variable for echo pin of ULTRASONIC
int button = 5;           // Variable for relaxation button
int buttonNew;            // button status
int buttonold = 1;        //button status
int ledblue = 7;          // Variable for blue LED(button)
int ledstat = 0;          // red led status

int ctr = 0;            // variable for counter for giving 1 relaxation period per day only

long duration;          // time duration for transmission and reception of ultrasonic wave
int distance;           // distance between object and ultrasonic sensor


SoftwareSerial esp8266(RX,TX);       // using function to make esp01 recognise rx and tx pin of arduino

/* In void setup we will be first initiating serialport by Serial.begin function at baud rate of 9600.
  esp01 receives and transmits data at baud rate of 115200.In dhtObject.setup(6),the arduino digital pin to which dht is connected
  is mentioned in the function.Further by using pinMode() function pins of ultrasonic sensors(triger,echo),LEDS,Buzzer,Button are
  declared as INPUT or OUTPUT as per their role.

  AT command returns OK which implies that the communication
  between esp8266 and the application has been verified.
  
  AT-CW command is used to set the WiFi Mode of operation as either Station mode,
  Soft Access Point (AP) or a combination of Station and AP. 

 AT-CWJAP
 This command is to connect to an Access Point (like a router).

*/
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  dhtObject.setup(6);
  pinMode(led, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenled, OUTPUT);
  pinMode(button, INPUT);
  pinMode(ledblue, OUTPUT);
  sendCommand("AT", 1, "OK");                          
  sendCommand("AT+CWMODE=1", 1, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
  delay(120000);
}

/*In void loop(),first status of Relaxation button is checked, if button is ON
  relaxation time of 25minutes is allowed.  Counter is used so that the relaxation time is allowed once per day.
*/

void loop()
{
  buttonNew = digitalRead(button);    //check button status
  Serial.println(buttonNew);
  if (buttonold == 0 && buttonNew == 1 && ctr == 0)  // if button is pressed and counter=1
  {
    if (ledstat == 0)
    {
      digitalWrite(ledblue, HIGH);          // Blue led will glow for 25minutes to showits relaxation time


      ledstat = 1;
    }
    delay(30000);                             //delay means no leds,no sensor detection,no buzzer for 25minutes

    /*Now after 25minutes, sensors will start reading data and leds and buzzer will work accordingly*/
    float humidity = dhtObject.getHumidity();      //read humidity data
    float temperature = dhtObject.getTemperature();  //red temperature data
    Serial.print(dhtObject.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);                //serial print humidity values
    Serial.print("\t\t");
    Serial.print(temperature, 1);             //serial print temperature in degree Celsius values
    Serial.print("\t\t");
    Serial.println(dhtObject.toFahrenheit(temperature), 1);  // serial print temperature in Fahrenheit values
    delay(1000);
    if (humidity < 74)   //if mask is worn off humidity will go low and if gone below 74, led red led will start blinking an buzzer will beep
    {
      digitalWrite(led, HIGH);
      delay(300);
      digitalWrite(led, LOW);
      digitalWrite(buzzPin, HIGH);
      delay(1000);
      digitalWrite(buzzPin, LOW);
      //delay(10000);
      if (humidity < 65)  //However if value comes to as low as 65, then data will be sent to thingspeak cloud
      {
        DATAthingspeak();
        digitalWrite(led, HIGH);
        delay(300);
        digitalWrite(led, LOW);
        digitalWrite(buzzPin, HIGH);
        delay(1000);
        digitalWrite(buzzPin, LOW);
      }
    }

    else if (humidity > 74) //till humidity value is above 74, no led and no buzzer
    {
      digitalWrite(led, LOW);
      digitalWrite(buzzPin, LOW);
    }
    /* an ultrasonic pulse is triggered and recepted through echo pin so as to ensure safe distance*/
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH); //duration is calculated by in_built function
    distance = duration * 0.034 / 2;  //distance is calculated
    Serial.print("Distance: ");
    Serial.println(distance);
    if (distance < 70)    //if distance is less than 70cm ,greenled will blink and buzzer will beep
    {
      digitalWrite(greenled, HIGH);
      delay(300);
      digitalWrite(greenled, LOW);
      digitalWrite(buzzPin, HIGH);
      delay(1000);
      digitalWrite(buzzPin, LOW);

    }
    else if (distance > 70)  // else greenled will be OFF and buzzer will also be OFF
    {
      digitalWrite(buzzPin, LOW);
      digitalWrite(greenled, LOW);
    }
    ctr += 1;             //NOW HERE AS WE HAVE USED Button Counter will incremented so that if button is again pressed it wont go in
                          //this loop again
  }
  /*ELSE---> MEANS IF RELAXATION BUTTON IS NOT PRESSED,then The entire process of DHT sensor,Ultrasonic sensor will again be repeated */
  else                
  {
    digitalWrite(ledblue, LOW);  //as no button pressed no blueled
    ledstat = 0;                      
    float humidity = dhtObject.getHumidity();
    float temperature = dhtObject.getTemperature();
    Serial.print(dhtObject.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.println(dhtObject.toFahrenheit(temperature), 1);
    delay(1000);
    if (humidity < 74)   
    {
      digitalWrite(led, HIGH);
      delay(300);
      digitalWrite(led, LOW);
      digitalWrite(buzzPin, HIGH);
      delay(1000);
      digitalWrite(buzzPin, LOW);
      //delay(30000);
      if (humidity < 65)   
      {
        DATAthingspeak();
        digitalWrite(led, HIGH);
        delay(300);
        digitalWrite(led, LOW);
        digitalWrite(buzzPin, HIGH);
        delay(1000);
        digitalWrite(buzzPin, LOW);
      }
    }

    else if (humidity > 74)   
    {
      digitalWrite(led, LOW);
      digitalWrite(buzzPin, LOW);
    }

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.print("Distance: ");
    Serial.println(distance);
    if (distance < 70)
    {
      digitalWrite(greenled, HIGH);
      delay(300);
      digitalWrite(greenled, LOW);
      digitalWrite(buzzPin, HIGH);
      delay(1000);
      digitalWrite(buzzPin, LOW);

    }
    else if (distance > 70)
    {
      digitalWrite(buzzPin, LOW);
      digitalWrite(greenled, LOW);
    }
  }
  buttonold = buttonNew;


}
/*AT+CIPMUX AT command configures the device for a single or multi IP connection.
  AT+CIPSTART comamnds starts a TCP or UDP connection between ESP01 and Arduino Uno.
  AT+CIPSEND AT command is used to send the data over the TCP or UDP connection.
  AT+CIPCLOSE At command closes the TCP or UDP connection. 
*/
void DATAthingspeak()
{
  //Here first data is extracted from dht11 sensor and then converted in string type.
  String getData = "GET /update?api_key=" + API + "&field1=" + getTemperatureValue() + "&field2=" + getHumidityValue();
  sendCommand("AT+CIPMUX=1", 1, "OK");    
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK"); //here TCP is established between esp01 and thingspeak server
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 1, ">");   //here data which is in string type is sent over Thingspeak
  esp8266.println(getData);                                              
  delay(1500);
  countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 1, "OK");     //after sending data over thingspeak tcp connection is closed
}


String getTemperatureValue() {

  //delay(dhtObject.getMinimumSamplingPeriod());
  float temperature = dhtObject.getTemperature();
  Serial.print(" Temperature(C)= ");
  Serial.println(temperature);
  return String(temperature);

}


String getHumidityValue() {

  //delay(dhtObject.getMinimumSamplingPeriod());
  float humidity = dhtObject.getHumidity();
  Serial.print(" Humidity in %= ");
  Serial.println(humidity);
  return String(humidity);

}


void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);//at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("CONNECTED");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}
