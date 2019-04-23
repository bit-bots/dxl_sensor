/*
 * This code is originally based on the work of Team Rhoban available at https://github.com/Rhoban/DXLBoard
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "dxl.h"

// adjust these values to your use case
ui8 DXL_ID = 100;
int baud = 4000000;

// buffer for dynamxiel bus
const int BUFFER_SIZE = 128;
ui8 write_buffer[BUFFER_SIZE];

int direction_pin = PB1;

volatile struct dxl_packet packet;
volatile struct dxl_packet ping_resp;
volatile struct dxl_packet read_resp;


void setup() {    

    //dynamixel connection
    Serial3.begin(baud);  
    pinMode(direction_pin, OUTPUT);
    digitalWrite(direction_pin, LOW);

    dxl_packet_init(&packet);
    dxl_packet_init(&ping_resp);
    dxl_packet_init(&read_resp);

    //create a ping resonse package which will always be the same    
    ping_resp.id = DXL_ID;
    ping_resp.instruction = 0x55;
    // set to model number 5005 and firmware 1
    ping_resp.parameters[0] = 0x8D;
    ping_resp.parameters[1] = 0x13;
    ping_resp.parameters[2] = 0x01;
    ping_resp.parameter_nb = 3;
    
    read_resp.id = DXL_ID;
    read_resp.instruction = 0x55;
    read_resp.parameter_nb = 12;
}

void sensor_tick() {
    // read your sensor here and save it into a variable

}


void bus_tick(){
  // read as many bytes as there are currently in the serial buffer
  while(Serial3.available() > 0){
    ui8 inByte = Serial3.read();
    dxl_packet_push_byte(&packet, inByte);
    if(packet.process){
      // we recieved a complete package, check if it is for us
      if(packet.id == DXL_ID){
        //check if it is a ping or a read
        if(packet.instruction == 1){
          response_to_ping();
        }else if(packet.instruction == 2){
          response_to_read();
        }
      }
      packet.process = false;
    }
  }  
}


void response_to_ping(){
  digitalWrite(direction_pin, HIGH);
  int len = dxl_write_packet(&ping_resp, write_buffer);
  Serial3.write(write_buffer, len);
  // make sure to write the complete buffer before changing direction pin  
  Serial3.flush();
  digitalWrite(direction_pin, LOW);
}

void response_to_read(){
  digitalWrite(direction_pin, HIGH);
  //make package with current bytes from the sensors
  read_resp.parameters[0]  =  0; // write your saved sensor readings in the response parameters 

  int len = dxl_write_packet(&read_resp, write_buffer);
  Serial3.write(write_buffer, len);
  Serial3.flush();
  digitalWrite(direction_pin, LOW);
}


void loop() {
  sensor_tick();
  bus_tick();
}
