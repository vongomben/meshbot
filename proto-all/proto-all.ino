#include <Meshtastic.h>

// Pin configuration for XIAO ESP32S3
#define SERIAL_RX_PIN 44   // GPIO44 (RX)
#define SERIAL_TX_PIN 43   // GPIO43 (TX)
#define BAUD_RATE 9600

// Interval between node report requests (ms)
#define NODE_REPORT_PERIOD (30 * 1000)

// Buffer for node data
mt_node_t node_infos[100];
uint8_t node_infos_count = 0;

// Timer for periodic node reports
uint32_t next_node_report_time = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Starting Meshtastic + text client for XIAO ESP32S3");

  // Initialize serial communication to Meshtastic radio
  mt_serial_init(SERIAL_RX_PIN, SERIAL_TX_PIN, BAUD_RATE);

  // (Optional) enable debug if supported
  // mt_set_debug(true);

  // Register the callback for text messages
  set_text_message_callback(text_message_callback);
}

// Callback for text messages
void text_message_callback(uint32_t from, uint32_t to, uint8_t channel, const char* text) {
  Serial.println("Message received:");
  Serial.print(" From: ");
  Serial.print(from);
  Serial.print(" → To: ");
  Serial.print(to);
  Serial.print(" | Channel: ");
  Serial.print(channel);
  Serial.print(" | Text: ");
  Serial.println(text);
}

// Callback for node report
void node_report_callback(mt_node_t * nodeinfo, mt_nr_progress_t progress) {
  if (progress == MT_NR_IN_PROGRESS && nodeinfo) {
    node_infos[node_infos_count++] = *nodeinfo;
  } else if (progress == MT_NR_DONE) {
    print_node_infos();
    node_infos_count = 0;
  } else if (progress == MT_NR_INVALID) {
    Serial.println("Invalid node response.");
  }
}

// Print node info
void print_node_infos() {
  Serial.print("Known nodes: ");
  Serial.println(node_infos_count);

  for (uint8_t i = 0; i < node_infos_count; i++) {
    mt_node_t* nodeinfo = &node_infos[i];

    Serial.print("Node ");
    Serial.print(nodeinfo->node_num);
    Serial.print(" (");
    Serial.print(nodeinfo->user_id);
    Serial.print(")");

    if (nodeinfo->is_mine) Serial.print(" [this node]");

    if (nodeinfo->has_user) {
      Serial.print(" | User: ");
      Serial.print(nodeinfo->long_name);
      Serial.print(" (");
      Serial.print(nodeinfo->short_name);
      Serial.print(")");
    }

    if (!isnan(nodeinfo->latitude)) {
      Serial.print(" | Location: ");
      Serial.print(nodeinfo->latitude, 6);
      Serial.print(", ");
      Serial.print(nodeinfo->longitude, 6);
      Serial.print(" | Alt: ");
      Serial.print(nodeinfo->altitude);
      Serial.print("m");
    }

    if (!isnan(nodeinfo->voltage)) {
      Serial.print(" | Battery: ");
      Serial.print(nodeinfo->voltage);
      Serial.print("V (");
      Serial.print(nodeinfo->battery_level);
      Serial.print("%)");
    }

    Serial.println();
  }
}

void loop() {
  uint32_t now = millis();
  bool can_send = mt_loop(now);

  if (can_send && now >= next_node_report_time) {
    mt_request_node_report(node_report_callback);
    next_node_report_time = now + NODE_REPORT_PERIOD;
  }

  delay(10);
}
