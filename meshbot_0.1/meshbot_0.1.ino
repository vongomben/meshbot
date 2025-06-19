// Meshbot v 0.1 experiments
// Davide Gomba aka vongomben
// the device is a Xiao ESP32S3 and
// is receiving via serial from
// a Seeed Xiao ESP32S3 Meshtastic kit
// 
// It's doing things based on commands

String Meshbot_version = "0.1\n";
String welcome = "Meshbot version " + Meshbot_version;
// Micro Servo Setup

#include <ESP32Servo.h>

Servo myservo;

int pos = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
int servoPin = D2;
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
int servoPin = 7;
#else
int servoPin = 18;
#endif


// DHT setup

#include "Grove_Temperature_And_Humidity_Sensor.h"

#define DHTTYPE DHT20  // DHT 20

DHT dht(DHTTYPE);  //   DHT10 DHT20 don't need to define Pin

#if defined(ARDUINO_ARCH_AVR)
#define debug Serial

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#define debug SerialUSB
#else
#define debug Serial
#endif

// Chainled Setup

#include <ChainableLED.h>

#define NUM_LEDS 1

ChainableLED leds(D0, D1, NUM_LEDS);  // works
// Meshtastic Setup

#include <Meshtastic.h>

// Pins used on the Xiao ESP32S3
#define SERIAL_RX_PIN 44  // GPIO44 (RX)
#define SERIAL_TX_PIN 43  // GPIO43 (TX)
#define BAUD_RATE 9600

#define SEND_PERIOD 300  // (optional) send every 300 seconds

uint32_t next_send_time = 0;
bool not_yet_connected = true;

//Global Variables for
uint32_t dest = BROADCAST_ADDR;
uint8_t channel = 0;

// Callback when connected to a Meshtastic node
void connected_callback(mt_node_t* node, mt_nr_progress_t progress) {
  if (not_yet_connected)
    Serial.println("Connected to Meshtastic device!");
  not_yet_connected = false;
}

// Callback when a text message is received
void text_message_callback(uint32_t from, uint32_t to, uint8_t channel, const char* text) {

  Serial.print("Received message:\n From: ");
  Serial.print(from);
  Serial.print(" → To: ");
  Serial.print(to);
  Serial.print(" | Channel: ");
  Serial.print(channel);
  Serial.print(" | Text: ");
  Serial.println(text);

  // Bot management: /help, /servo, /red /temp
  if (strcmp(text, "/help") == 0) {
    Serial.println("this is help");

    String welcome = "Meshbot version " + Meshbot_version;

    Serial.println(welcome);

    String message = welcome + "type \"/servo\" to control servo\n" + "type \"/red\" to control LED\n" + "type \"/temp\" to read temperature!";

    mt_send_text(message.c_str(), dest, channel);
  }
  // Bot management: /servo
  if (strcmp(text, "/servo") == 0) {
    Serial.println("this is servo");

    for (pos = 0; pos <= 180; pos += 1) {  // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);  // tell servo to go to position in variable 'pos'
      delay(15);           // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
      myservo.write(pos);                  // tell servo to go to position in variable 'pos'
      delay(15);                           // waits 15ms for the servo to reach the position
    }


    mt_send_text("this is servo\n", dest, channel);
  }
  // Bot management: /red
  if (strcmp(text, "/red") == 0) {
    Serial.println("this is red");
    blinkTimes(10);
    mt_send_text("this is red\n", dest, channel);
  }
  // Bot management:  /temp
  if (strcmp(text, "/temp") == 0) {
    Serial.println("this is temp");

    float temp_hum_val[2] = { 0 };

    if (!dht.readTempAndHumidity(temp_hum_val)) {
      float hum = temp_hum_val[0];
      float temp = temp_hum_val[1];
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println(" *C");

      String message = "temp is " + String(temp) + " hum is " + String(hum);

      mt_send_text(message.c_str(), dest, channel);

    } else {
      debug.println("Failed to get temperature and humidity value.");
    }
  }
}


void setup() {
  Serial.begin(9600);   // USB for debugging
  Serial1.begin(9600);  // Hardware port for receiving from another board (if used)

  // Initialize serial communication with the Meshtastic device
  mt_serial_init(SERIAL_RX_PIN, SERIAL_TX_PIN, BAUD_RATE);
  mt_set_debug(true);  // set to true for detailed output

  // Request initial node info from the Meshtastic device
  mt_request_node_report(connected_callback);

  // Register callback for received messages
  set_text_message_callback(text_message_callback);

  // broadcast prosperity

  String broadcast = "Ciao! - Meshbot version " + Meshbot_version;
  mt_send_text(broadcast.c_str(), dest, channel);

  // DHT setup

  Wire.begin();
  dht.begin();

  // Servo Setup

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);            // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);  // attaches the servo on pin 18 to the servo object
                                         // using default min/max of 1000us and 2000us
                                         // different servos may require different min/max settings
                                         // for an accurate 0 to 180 sweep
}

void loop() {

  // first: turn LED off
  leds.setColorRGB(0, 0, 0, 0);  // set the only LED to black

  uint32_t now = millis();
  bool can_send = mt_loop(now);  // processes incoming messages


  /*
  // (optional) Send a message every few seconds
  if (can_send && now >= next_send_time) {
    //uint32_t dest = BROADCAST_ADDR;
    //uint8_t channel = 0;
    mt_send_text("Hello from Meshbot!", dest, channel);
    next_send_time = now + SEND_PERIOD * 1000;
  }
*/
}

// controlling chainable LED

void blinkTimes(int times) {

  for (int i = 0; i < times; i++) {

    leds.setColorRGB(0, 255, 0, 0);
    delay(100);
    leds.setColorRGB(0, 0, 0, 0);
    delay(100);
  }
}
