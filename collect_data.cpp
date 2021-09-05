#include <iostream>    // cin, cout, endl
#include <string>      // string, getline()
#include <chrono>      // timing
#include <RF24/RF24.h> // RF24
#include <unistd.h>    // Sleep()
#include <sqlite3.h>

using namespace std;

#define PIN_CE  17
#define PIN_CSN 0
#define INST_LENGTH 10
#define TIMEOUT_LENGTH 1000
// timeout length in milliseconds

uint8_t addresses[][6] = {"0Node", "1Node", "2Node", "3Node"}; 
// Radio pipe addresses for the 2 nodes to communicate.
// address is a path instead of as an identifying device destination

uint8_t instructions[][INST_LENGTH] = {"Get_Temp", "Moisture", "Light"};
uint8_t pipeNumber;
uint8_t payloadSize;
	
RF24 radio(PIN_CE, PIN_CSN);

// -----------------------------------------------------------------

void sendInstruction(int i) {
  bool report = radio.write(&instructions[i], sizeof(instructions[i]));
  if (!report) {
    cout << "Failed to send instruction" << endl;
  }
}

// Get various measurments from sensor

/* Asks for and gets the light intensity in lux */
float getLightIntensity() {
  radio.stopListening();
  float rxLux = 0.0;
  unsigned long start_timeout = millis();
  sendInstruction(2);

  radio.startListening();
  while (millis() - start_timeout < TIMEOUT_LENGTH) {
    if (radio.available(&pipeNumber)) {
		payloadSize = radio.getDynamicPayloadSize();
        radio.read(&rxLux, payloadSize);
		
		cout << "Recieved " << +payloadSize << " bytes on pipe " << +pipeNumber << endl;
		
        return rxLux;
    }
  }
  cout << "Timeout reached" << endl;
  return rxLux;
}

/* Asks for an returns the temp in C */
float getTemp() {
  radio.stopListening();
  float rxTemp = 0.0;
  unsigned long start_timeout = millis();

  sendInstruction(0);

  radio.startListening();
  while (millis() - start_timeout < TIMEOUT_LENGTH) {
    if (radio.available(&pipeNumber)) {
		payloadSize = radio.getDynamicPayloadSize();
        radio.read(&rxTemp, payloadSize);
		
		cout << "Recieved " << +payloadSize << " bytes on pipe " << +pipeNumber << endl;

        return rxTemp;
    }
  }
  cout << "Timeout reached" << endl;
  return rxTemp;
}

/* Asks for an returns the moisture level */
int getMoisture() {
  radio.stopListening();
  int moisture = 0;
  unsigned long start_timeout = millis();

  sendInstruction(1);

  radio.startListening();
  while (millis() - start_timeout < TIMEOUT_LENGTH) {
    if (radio.available(&pipeNumber)) {
		payloadSize = radio.getDynamicPayloadSize();
        radio.read(&moisture, payloadSize);
		
		cout << "Recieved " << +payloadSize << " bytes on pipe " << +pipeNumber << endl;
		
        return moisture;
    }
  }
  cout << "Timeout reached" << endl;
  return moisture;
}

// ----------------------------------------

void debugLoop() {
	
    while(true) {

        float temp = 0.0;
        int moisture = 0;
        float light = 0.0;

        cout << "Getting Temp" << endl;
        temp = getTemp();
		cout << endl;
        delay(1000);
        cout << "Getting Moisture" << endl;
        moisture = getMoisture();
		cout << endl;
        delay(1000);
        cout << "Getting Light" << endl;
        light = getLightIntensity();
		cout << endl;


        cout << "Temp: " << temp << " C | Moisture: " << moisture << " | Light: " << light << " lux" << endl;
		cout << endl;
        delay(2000);
    }
}


// ---------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  sqlite3* DB;
  int exit = sqlite3_open("./data/sensor_data.db", &DB);

  if (exit) {
    std::cerr << "Error opening DB " << sqlite3_errmsg(DB) << std::endl;
    return (-1);
  } else {
    std::cout << "Opened Database Successfully!" << std::endl;
  }


    if(!radio.begin()) {
        cout << "radio harware is not responding";
        return 0;
    }
	
	radio.setChannel(0x5F);
	radio.setDataRate (RF24_1MBPS);
	radio.setPALevel(RF24_PA_HIGH);
	radio.enableDynamicPayloads();
	
	
	cout << radio.txDelay <<endl;
	
	radio.printPrettyDetails();
	
	


    // TODO: save on transmission time by setting payload size

    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
	radio.openReadingPipe(2, addresses[2]);
	radio.openReadingPipe(3, addresses[3]);
    radio.startListening();

    debugLoop();

    return 0;
}
