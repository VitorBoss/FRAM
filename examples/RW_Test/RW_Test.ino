#include <Fram.h>

#if defined(STM32_CORE_VERSION)
  SPIClass spiToUse(PB5, PB4, PB3, PB0); /* SPIClass(mosi, miso, sclk, ssel = (uint8_t)NC); */
#elif defined(STM32GENERIC)
  SPIClass spiToUse(PB5, PB4, PB3); /* SPIClass(mosi, miso, sck) */
#else defined(_VARIANT_ARDUINO_STM32_) //libmaple core
  SPIClass spiToUse(1);
#endif
FramClass Fram(PB5, PB4, PB3, PB0); /* FramClass(mosi, miso, sclk, ssel = (uint8_t)NC); */
//FramClass Fram(PB0, spiToUse);

void setup()
{
  Serial.begin(115200);
  delay(50); //ARM devices need an extra time
  //Fram.setClock(10000000); /* 10MHz */
  Serial.print(("Status Register : 0x"));
  Fram.EnableWrite(true);
  Serial.println(Fram.readSR(), HEX);
}

void loop()
{
  if(Serial.available() > 0)
  {
    switch(Serial.read())
    {
    case 'Q':
    case 'q':
      {
        Serial.print(("Status Register : 0x"));
        Serial.println(Fram.readSR(), HEX);
      }
      break;
    case 'R':
    case 'r':
      {
        //R0,100
        Serial.print(("Read "));
        int addr = Serial.parseInt();
        int len = Serial.parseInt();
        if (len == 0) len++;
        Serial.print(("addr=0x"));
        Serial.print(addr,HEX);
        Serial.print((", len=0x"));
        Serial.print(len,HEX);
        Serial.println((":"));
        uint8_t *buf = new uint8_t[len];
        Fram.read(addr,buf,len);
        for(int i = 0;i < len; i++)
        {
          Serial.print((char)buf[i]);
        }
        Serial.println();
        for(int i = 0;i < len; i++)
        {
          Serial.print((char)buf[i], HEX);
        }
        Serial.println();
        Serial.println(("OK"));
        delete [] buf;
      }
      break;
    case 'W':
    case 'w':
      {
        //W0,test string[LF]
        //
        Serial.print(("Write "));
        int addr = Serial.parseInt();
        Serial.print(("0x"));
        Serial.print(addr,HEX);
        Serial.read();
        uint8_t *buf = new uint8_t[256];
        uint8_t len = Serial.readBytesUntil('\n',(char*)buf,256);
        Serial.print((",0x"));
        Serial.print(len,HEX);
        Serial.print((": "));
        Fram.EnableWrite(1);
        Fram.write(addr,buf,len);
        Serial.println(("OK"));
        delete [] buf;
      }
      break;
    case 'E':
    case 'e':
      {
        //e4096
        uint8_t *buf = new uint8_t[256];
        for(uint8_t x = 0; x<255; x++) { buf[x]= 255; }
        int len = Serial.parseInt();
        int addr = 0;
        int i,n;
        if(len==0) len = 255;
        n=len;
        Fram.EnableWrite(1);
        Serial.println(("Erasing..."));
        for(i = 0; i < 128; i++) 
        {
           len-=255;
           Fram.write(addr+(256*i),buf,(len<255? 255:len));
           if(len<255) break;
        }
        Serial.print(("Erased ")); Serial.print(n); Serial.println((" bytes."));
        Serial.println(("OK"));
        delete [] buf;
      }
      break;
    }
  } 
}


