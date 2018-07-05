const uint8_t tableHi[] = 
{0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t tableLo[] = 
{0xF1,0x80,0x13,0xAD,0x4D,0xF3,0x9D,0x4D,0x00,0xB8,0x75,0x35,0xF8,0xBF,0x89,0x56,0x26,0xF9,0xCE,0xA6,0x7F,0x5C,0x3A,0x1A,0xFB,0xDF,0xC4,0xAB,0x93,0x7C,0x67,0x52,0x3F,0x2D,0x1C,0x0C,0xFD,0xEF,0xE2,0xD2,0xC9,0xBD,0xB3,0xA9,0x9F,0x96,0x8E,0x86,0x7E,0x77,0x70,0x6A,0x64,0x5E,0x59,0x54};
                                                     //These tables were taken from http://www.freewebs.com/the_bott/NotesTableNTSC.txt
                                                     //tableHi is the top 3 bits for $4003, tableLo is the bottom 8 bits for $4002

volatile boolean state = false;                      //switch to make sure I write values at the right time

void writeData(){                                    //routine to switch state while NES is BRKing
  state=true;                                        //switch state
}

void sendAddrData(uint8_t address, uint8_t data){    //routine to send address and data along to '245s when interrupt is thrown
  state = false;                                     //set switch
  while (state!=true);                               //wait for interrupt to throw switch back
  PORTF = address;                                   //write address variable to register
  PORTK = data;                                      //write data variable to other register
  delayMicroseconds(3);                              
/*I have no clue why, but I have to include this delay somewhere in 
this routine, else the sound will be messed up. It works literally anywhere. The reason I added this in the first place is 
lost to me. It came as one of those 'flash of inspiration' moments, like when you're puzzling over something at 3am, and a 
little voice tells you to insert this piece of code here, and it works. idk lol. compiler optimization error?*/
}

void setup() {
  DDRF = 0xFF;                                       //initialize registers as output
  DDRK = 0xFF;
  PORTF= 0x15;                                       //Good initial values
  PORTK= 0x0F;  
  pinMode(49, OUTPUT);                               //pin 49 is connected to the NES +5V pin through TIP120
  pinMode(53, OUTPUT);                               //pin 53 is connected to the NES /RST pin
  digitalWrite(49, LOW);                             //Turn power to NES CPU off
  digitalWrite(53, LOW);                             //Bring /RST low
  delay(500);                                        //wait .5 seconds
  digitalWrite(49,HIGH);                             //turn on NES CPU
  delay(500);                                        //wait .5 seconds
  digitalWrite(53,HIGH);                             //bring /RST pin high
  attachInterrupt(2, writeData, RISING);             //begin interrupt function on pin 21, calls writeData on falling edge
}

void loop() {                                        //main loop
  sendAddrData(0x15, 0x1F);                          //Turn on all channels
  sendAddrData(0x00, 0xBF);                          //Turn square wave to max volume, 50% duty, etc
  sendAddrData(0x01, 0x08);                          //Negate sweeps
  for(int n; n < sizeof(tableHi)/sizeof(int); n++)
  {
    sendAddrData(0x02, tableLo[n]);                  //increment through the note table posted above, low bits
    sendAddrData(0x03, tableHi[n]);                  //increment through the note table posted above, high bits
    sendAddrData(0x1F, 0x00);                        //end write, NES does its thing now
    delay(125);                                      //note duration
  }
  sendAddrData(0x15, 0x00);                          //silence all channels
  sendAddrData(0x1F, 0x00);                          //end write
}
