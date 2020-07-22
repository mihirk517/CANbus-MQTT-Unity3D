#include <mcp_can.h>
#include <SPI.h>
#include<Wire.h>
#define EXTEND A1
#define RETRACT A3 
#define STOP A0
#define slaveAddress 9
#define SPI_CS_PIN  10
#define extendedFrame 1
#define Length 8
byte msg;
char input=0;
long unsigned int txID = 0x18EFC8C8 ;
char load[8] = {0xF4, 0x01, 0xFB, 0xFB, 0xFB, 0xFB, 0xFF, 0xFF}; 
char unload[8] = {0x02, 0xFB, 0xFB, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF};
char error[8] = {0x00, 0xFB, 0xFB, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF};
char estop[8]= {0x03, 0xFB, 0xFB, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF};
//char pos[8]= {0xB1, 0xB2, 0xFB, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF};
enum States {LOAD,UNLOAD,STANDBY,ERROR_CLEAR,ESTOP};
States currentState=ERROR_CLEAR;
unsigned char len = 0;
unsigned char buf[8];
long unsigned int rxId;
unsigned char lenR = 0;
unsigned char rxBuf[8];
unsigned char command;
char msgString[128]; 
bool stateL,stateU=false;
unsigned long Pos;
String pos;
float Position;
char POSITION[5];


MCP_CAN CAN0(SPI_CS_PIN); 
float check_position(float Position){
  CAN0.readMsgBuf(&rxId, &lenR, rxBuf); // Read data: len = data length, buf = data byte(s)
     
     for(byte i = 0; i<lenR; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Pos = (rxBuf[1] << 8) + rxBuf[0];
        pos =  String(Pos, DEC);
        Position = (pos.toFloat())/10;
        return Position;
           
}
}
bool reciever(byte Byte1,byte Byte2, bool stat)  {
    CAN0.readMsgBuf(&rxId, &lenR, rxBuf); // Read data: len = data length, buf = data byte(s)
     
     for(byte i = 0; i<lenR; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        //Serial.print(msgString);
        
        if (rxBuf[2]==0x00 && rxBuf[0]== Byte1 && rxBuf[1]== Byte2 ){  
          stat=true;              
          }                  
          else stat=false;       
          Pos = (rxBuf[1] << 8) + rxBuf[0];
          pos =  String(Pos, DEC);
          Position = (pos.toFloat())/10;     
          }
          return stat;
          
           
}
void setup()
{
      Serial.begin(115200);
      pinMode(EXTEND,INPUT_PULLUP);
      pinMode(RETRACT,INPUT_PULLUP);
      pinMode(STOP,INPUT_PULLUP);

  // Initialize MCP2515 running at 8MHz with a baudrate of 250kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
      Wire.begin(slaveAddress);
      Wire.onReceive(recieveEvent);
      Wire.onRequest(requestEvent);
      Serial.println("Slave Ready");
  
  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}


void loop()
{        

    switch(currentState){
          case LOAD:
          Serial.println("LOAD");
           if (digitalRead(STOP)== HIGH){          
          msg = CAN0.sendMsgBuf(txID,extendedFrame, 8, load);
          Serial.println(check_position(Position));        
          stateL=reciever(0xF4,0x01,stateL);    
          if (stateL ==true){           
          currentState=ERROR_CLEAR;   
          }
           }
           else
           currentState=ESTOP;              
          break;
                       
          case UNLOAD:
          Serial.println("UNLOAD");
          if (digitalRead(STOP)== HIGH){
          msg = CAN0.sendMsgBuf(txID,extendedFrame, Length, unload);
          Serial.println(check_position(Position)); 
          stateU = reciever(0x00,0x00,stateU);
           if ( stateU ==true){    
          currentState=ERROR_CLEAR;
          }
          }
          else
          currentState=ESTOP;          
          break;
                    
          case STANDBY:   
          if(digitalRead(RETRACT)== LOW ){currentState=UNLOAD;}
          if(digitalRead(STOP)== LOW ){currentState=ESTOP;}
          if(digitalRead(EXTEND)== LOW ){currentState=LOAD;}          
                 
          break;
          
          case ESTOP:          
           msg = CAN0.sendMsgBuf(txID,extendedFrame, Length, estop);
           Serial.println("Estop");       
          currentState=ERROR_CLEAR;
          break;
          
          case ERROR_CLEAR:          
           msg = CAN0.sendMsgBuf(txID,extendedFrame, Length, error);
           Serial.println("ERROR");
           Serial.println(check_position(Position));                      
          currentState=STANDBY;
          break;
          }
}
void recieveEvent(){
  if(Wire.available()>0)
  {
    command = Wire.read();
  }
  Serial.print("Recieved Message:");
  Serial.println(command);
  if (command==2){Serial.println("LOAD"); currentState=LOAD;}
  if (command==3){Serial.println("UNLOAD"); currentState=UNLOAD;}
  if (command==4){Serial.println("ESTOP"); currentState=ESTOP;}
 }
 void requestEvent(){  
  dtostrf(Position,3,2, POSITION);
  Wire.write(POSITION,sizeof(POSITION));  
 }
