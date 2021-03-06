/************************************************************************** 
|PC| <-serial-> |Moteino_Gw| <-RFM69W-> |Target_Moteino|

NOTE: Endian Conversion Required to properly read encoded data
https://docs.python.org/2/library/struct.html
Linux = Little Endian (Python conversion table)
Moteino =  Big endian
Data Type Cross Reference Table
--------------------------------
Type	Linux(python)	Moteino
byte 	char(1)			byte(1)
int 	short(2)		int(2)
long 	long(4) 		long(4)
float 	float(4)		float(4)

***************************************************************************/


#include <RFM69.h>
#include <SPI.h>
#include <NodeGwMsg.h> // Load Gateway Message structure:Task
#include <NodeConf.h>
// Initialize Redio
RFM69 radio;

// Some shared variables.
byte buff[MAX_SERIAL_SIZE];
byte nrBuff[MAX_SERIAL_SIZE];
int moteReadLength;
// -- BEGIN Function Prototypes
void Blink(byte PIN, int DELAY_MS) {
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

// END Function Prototypes
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.encrypt(KEY);
}

void loop() {
	// READ Serial input if avaiable...
	if (Serial.available() > 0) {
		//Wait for serial buffer to fill
		delay(100);
		// Read serial data into buffer
		// METHOD 1 Blind read, no limit (Risky?)
		int count=0;

		// read until the serial buffer is empty
		while (Serial.available() > 0) {
			buff[count] = (byte)Serial.read();
			count++;
		}
		/*
		Serial.print("Count = ");
		Serial.println(count);
		*/
	 	// End read data from serial 
		// read Serial Message
		mMsg = *(SerialMsg*)buff;
		if (mMsg.SerialDelimiter == 0) {
			//memcpy(o_payload.msg, &mMsg.SerialPayload, sizeof(mMsg.SerialPayloadSize));
			//radio.sendWithRetry(mMsg.NodeID, (const void*)(&mMsg.SerialPayload), mMsg.SerialPayloadSize);
			radio.send(mMsg.NodeID, (const void*)(&mMsg.SerialPayload), mMsg.SerialPayloadSize);
			// good Serial Message Blink Yellow Light
			Blink(LED, 10);
		} 	
	}

	// Get incomming messages from Moteino Network
	// Repack message for Node-Red Serial gateway. 
	// Forward to gateway.
	if (radio.receiveDone() ) {
		// Store data in nMsg
		// Read data from radio
		
		payload = *(Payload*)radio.DATA;
		// Pack data 
		nrMsg.SerialDelimiter = 0x00;
		// Capture the source node address (sender)
		nrMsg.NodeID = radio.SENDERID;
		// Capture source message size
		nrMsg.SerialPayloadSize = radio.DATALEN; // Does not include 
		// Calculate message length, needed for transmission
		moteReadLength = SERIAL_HEADER_SIZE + radio.DATALEN;
		// Copy data from Radio to Serial Payload. 
		memcpy(nrMsg.SerialPayload, (const void*)(&radio.DATA), nrMsg.SerialPayloadSize);
		// Message Indicator
		Blink(LED, 10);
		// convert nrMsg structure to array for serial Tx
		memcpy(nrBuff, &nrMsg, sizeof(nrMsg));
		// Send data to Node-Red serial port		
		Serial.write(nrBuff, moteReadLength);
		
	}	
}