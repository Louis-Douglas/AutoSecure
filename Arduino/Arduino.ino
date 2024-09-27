#include <SoftwareSerial.h>
#include "LowPower.h"

const char serverIP[] =        "ServerIP";
const char IOT_CLIENT[] =      "Client";
const char IOT_USERNAME[] =    "Username";
const char IOT_KEY[] =         "Password";
const char IOT_TOPIC[] =       "Topic";

// Define the RX and TX pins for SoftwareSerial
const int RX_PIN = 8; // Arduino digital pin 0 to connect to the NB-IoT module's TX pin
const int TX_PIN = 7; // Arduino digital pin 1 to connect to the NB-IoT module's RX pin

// High = Sleep
// Low = Wake
const int SLEEP_PIN = 2; // Arduino digital pin 2 to connect to NB-IoT module's DTR pin



int counter = 2; // 450 * 8 = 1 hour of seconds


void flashShort() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

void flashLong() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}

float roundFloat(float number, int decimalPlaces) {
  float multiplier = pow(10, decimalPlaces);
  return round(number * multiplier) / multiplier;
}

class SerialModule {
  private:
    SoftwareSerial iotSerial;
    int RX;
    int TX;
    float previousLatitude = -1000.0;
    float previousLongitude = -1000.0;

    void decodeGPS(String data, String (&values)[4]) {
        // Split CSV data by commas
      int commaIndex = 0;
      int prevCommaIndex = 0;
      int valueIndex = 0;
      data += ",";
      
      for (int i = 0; i < data.length(); i++) {
        if (data.charAt(i) == ',' || data.charAt(i) == ':') {
          String value = data.substring(prevCommaIndex, commaIndex);

          switch (valueIndex) {
            case 1: // GNSS run status
              value.trim();
              values[0] = value;
              break;
            case 2: // Fix status
              values[1] = value;
              break;
            case 4: // Latitude
              values[2] = value;
              break;
            case 5: // Longitude
              values[3] = value;
              break;
            default:
              break;
          }
          valueIndex++;
          prevCommaIndex = i + 1;
        }
        commaIndex++;
      }
    }

  public:
    // Constructor
    SerialModule(int rxPin, int txPin) : RX(rxPin), TX(txPin), iotSerial(rxPin, txPin) {}
    
    // Initialize serial communication with NB-IoT module
    void begin(long baudRate) {
      iotSerial.begin(baudRate);
    }

    String sendATCommand(String cmd, bool printResponse) {
      return sendATCommand(cmd, "OK", printResponse, 4000);
    }

    String sendATCommand(String cmd, String expectedResponse, bool printResponse) {
      return sendATCommand(cmd, expectedResponse, printResponse, 4000);
    }

    String sendATCommand(String cmd, String expectedResponse, bool printResponse, unsigned int timeout) {
      unsigned long startTime = millis();
      Serial.print("Sending: "); 
      Serial.println(cmd); 
      iotSerial.println(cmd);
      String response = "";
      delay(100);
      iotSerial.listen();
      while (true) {
        // Check if timed out
        if (millis() - startTime > timeout) {
          Serial.println("ERROR: Timed out!");
          return "ERROR: Timeout";
        }

        while (iotSerial.available()) {
          char c = iotSerial.read();

          if (c != '\r' && c != '\n') {
            response += c;
          } else {
            break;
          }
          delay(5);
        }

        if (response.startsWith(expectedResponse)) {
          break;
        } else if (response != "") {
          if (printResponse) {
            Serial.print("Unexpected response: "); 
            Serial.println(response);
          }
          response = ""; // Clear the response string
          continue;
        }
      }
      iotSerial.flush();
      if (printResponse) {
        Serial.print("SIMCOM: "); Serial.println(response);
      }
      return response;
    }

    String listenForResponse(String expectedResponse, bool printResponse, unsigned int timeout) {
      unsigned long startTime = millis();
      String response = "";
      delay(100);
      iotSerial.listen();
      while (true) {
        // Check if timed out
        if (millis() - startTime > timeout) {
          Serial.println("ERROR: Timed out!");
          return "ERROR: Timeout";
        }

        while (iotSerial.available()) {
          char c = iotSerial.read();

          if (c != '\r' && c != '\n') {
            response += c;
          } else {
            break;
          }
          delay(5);
        }

        if (response.startsWith(expectedResponse)) {
          break;
        } else if (response != "") {
          if (printResponse) {
            Serial.print("Unexpected response: "); 
            Serial.println(response);
          }
          response = ""; // Clear the response string
          continue;
        }
      }
      iotSerial.flush();
      if (printResponse) {
        Serial.print("SIMCOM: "); Serial.println(response);
      }
      return response;
    }


    void setupMQTT() {
      // These shouldn't fail
      Serial.println("Setting up MQTT configuration...");
      sendATCommand("AT+SMCONF=URL," + String(serverIP) + ",1883", true);
      sendATCommand("AT+SMCONF=username," + String(IOT_USERNAME) + "", true);
      sendATCommand("AT+SMCONF=password," + String(IOT_KEY) + "", true);
      sendATCommand("AT+SMCONF=clientid," + String(IOT_CLIENT) + "", true);
      Serial.println("Sucessfully setup MQTT configuration!");
    }

    bool hasSignal() {
      int commaIndex = 0;
      int prevCommaIndex = 0;
      int valueIndex = 0;
      String values[2] = {"", ""};

      String data = sendATCommand("AT+CSQ", "+CSQ:",  true);
      
      for (int i = 0; i < data.length(); i++) {
        if (data.charAt(i) == ',' || data.charAt(i) == ':') {
          String value = data.substring(prevCommaIndex, commaIndex);

          switch (valueIndex) {
            case 1: // Signal strength (Hopefully above -100)
              values[0] = value;
              break;
            case 2: // Error rate (should be 0)
              values[1] = value;
              break;
            default:
              break;
          }
          valueIndex++;
          prevCommaIndex = i + 1;
        }
        commaIndex++;
      }
      return (values[0].toInt() > -100 && values[1].toInt() == 0);
    }

    void powerOnGPS() {
      sendATCommand("AT+CGNSPWR=1", true);
    }

    bool getGPS(String (&latlong)[2]) {
      String values[] = {"", "", "", ""};
      int retryAttempts = 5;
      for (int i = 0; i < retryAttempts; i++) {
        // Get GPS Location
        String response = sendATCommand("AT+CGNSINF", "+CGNSINF:", true);
        String values[4];

        // Call the function to fill the array
        decodeGPS(response, values);

        Serial.println("GPS: Run status: " + values[0]);
        Serial.println("GPS: Fix status: " + values[1]);
        Serial.println("GPS: Lat: " + values[2]);
        Serial.println("GPS: Long: " + values[3]);

        if (values[1] == "1" && values[2] != "" && values[3] != "") {
          latlong[0] = values[2];
          latlong[1] = values[3];
          flashLong();
          return true;
        }
        delay(10000);
      }
      Serial.println("Unable to get GPS location!");
      flashShort();
      return false;
    }

    bool hasMoved(String latlong[]) {
      float latitude = latlong[0].toFloat();
      float longitude = latlong[1].toFloat();

      // Round the coordinates to check for a larger amount of movement
      float correctedLat = roundFloat(latitude, 3);
      float correctedLong = roundFloat(longitude, 3);

      if (correctedLat != previousLatitude ||
          correctedLong != previousLongitude) {
            Serial.println("Vehicle has moved!");
            return true;
          }
      Serial.println("Vehicle has not moved!");
      return false;
    }

    void publishLocation(String latlong[]) {
      setNetworkActive(true);
      delay(1000);
      Serial.println("Logging into MQTT broker...");
      String response = sendATCommand("AT+SMCONN", "+APP PDP: ACTIVE", true, 10000);
      if (response != "OK") {
        Serial.println("ERROR: Cannot login to MQTT broker!");
        setNetworkActive(false);
        return;
      }
      delay(1000);
      Serial.println("Publishing location");

      String payload = String(IOT_CLIENT) + "," + latlong[0] + "," + latlong[1];
      
      iotSerial.print("AT+SMPUB=" + String(IOT_TOPIC) + ",\"" + payload.length() + "\",1,1\r\n");
      delay(500);
      iotSerial.println(payload);
      delay(500);
      iotSerial.write(0x1A); // send ctrl-z

      // Beta testing vvvv
      listenForResponse("OK", true, 10000);

      delay(1000);
      Serial.println("Logging out of MQTT broker...");
      sendATCommand("AT+SMDISC", true);
      delay(2000);
      setNetworkActive(false);
    }

    void setNetworkActive(bool activate) {
      if (activate) {
        Serial.println("Enabling local network!");
        sendATCommand("AT+CNACT=1,\"dataconnect.m2m\"", "OK", true);
      } else {
        Serial.println("Disabling local network!");
        sendATCommand("AT+CNACT=0", true);
      }
    }

    void sleep() {
      digitalWrite(SLEEP_PIN, HIGH);
    }

    void wake() {
      digitalWrite(SLEEP_PIN, LOW);
    }
};

  // Create a Serial Module object
SerialModule nbIoT(RX_PIN, TX_PIN);


void setup() {
  // Initialize serial communication with the Arduino IDE Serial Monitor
  Serial.begin(9600);
  delay(5000);
  // Initialize serial communication with the NB-IoT module
  Serial.println("Connecting to NB-IoT Module...");
  delay(100);
  nbIoT.begin(19200);
  Serial.println("Connected sucessfully to NB-IoT Module!");
  delay(100);
  // nbIoT.sendATCommand("AT+IPR=19200", true);
  // delay(200);
  nbIoT.powerOnGPS();
  delay(200);
  nbIoT.setupMQTT();
  delay(200);

  // // Enable longer error messages
  nbIoT.sendATCommand("AT+CMEE=2", "AT+CMEE:", true);
  delay(200);
  
}

void loop() {
  if (counter <= 0) {
    counter = 37; // 4.99 mins, 75 = 10 mins
    // nbIoT.wake();
    if (!nbIoT.hasSignal()) {
      Serial.println("No Signal");
      // nbIoT.sleep();
      return;
    }

    String latlong[] = {"", ""};
    if (!nbIoT.getGPS(latlong)) {
      return;
    }
    if (!nbIoT.hasMoved(latlong)) {
      return;
    }

    delay(2000);
    nbIoT.publishLocation(latlong);
    // nbIoT.sleep();
    delay(1000);
    Serial.println("Published location! Congrats!");
    delay(1000);
  }

  counter--;
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
