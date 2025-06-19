// Meshbot v 0.1 experiments
//
// the device is a Xiao ESP32S3 and
// is receiving via serial from
// a Seeed Xiao ESP32S3 Meshtastic kit
// and sending back the message

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
    mt_send_text("type \"/servo\" to control servo\n"
                 "type \"/red\" to control LED\n"
                 "type \"/temp\" to read temperature!",
                 dest, channel);
  }
  // Bot management: /servo
  if (strcmp(text, "/servo") == 0) {
    Serial.println("this is servo");
    mt_send_text("this is servo\n", dest, channel);
  }
  // Bot management: /red
  if (strcmp(text, "/red") == 0) {
    Serial.println("this is red");
    mt_send_text("this is red\n", dest, channel);
  }
  // Bot management:  /temp
  if (strcmp(text, "/temp") == 0) {
    Serial.println("this is temp");
    mt_send_text("this is temp\n", dest, channel);
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
}

void loop() {
  uint32_t now = millis();
  bool can_send = mt_loop(now);  // processes incoming messages

  // (optional) Send a message every few seconds
  if (can_send && now >= next_send_time) {
    //uint32_t dest = BROADCAST_ADDR;
    //uint8_t channel = 0;
    mt_send_text("Hello from Meshbot!", dest, channel);
    next_send_time = now + SEND_PERIOD * 1000;
  }
}
