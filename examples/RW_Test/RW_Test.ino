#include <Fram.h>

#if defined(STM32_CORE_VERSION) || defined(STM32GENERIC)
  SPIClass spiToUse(PB5, PB4, PB3); /* SPIClass(mosi, miso, sclk); */
#elif defined(_VARIANT_ARDUINO_STM32_) //libmaple core
  SPIClass spiToUse(0);
#endif
//FramClass Fram(PB5, PB4, PB3, PB0); //SoftSPI mode FramClass(mosi, miso, sclk, ssel = (uint8_t)NC);
//FramClass Fram(PB0);
FramClass Fram(PB0, spiToUse);

void setup()
{
  Serial.begin(115200);
  Fram.setClock(100000);//debug signals
#if defined (ARDUINO_ARCH_SAMD) || (__AVR_ATmega32U4__) || defined(ARDUINO_ARCH_STM32) || defined(NRF5)
  while (!Serial) ; // Wait for Serial monitor to open
#endif
  delay(150); //ARM devices need an extra time
  if( Fram.isDeviceActive() )
    Serial.println(F("Device found"));
  else
    Serial.println(F("Device not found, check wiring"));
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
        if( Fram.isDeviceActive() )
          Serial.println(F("Device found"));
        else
          Serial.println(F("Device not found, check wiring"));
      }
      break;

    case 'R':
    case 'r':
      {
        //R0,100
        Serial.print(F("Read "));
        int addr = Serial.parseInt();
        int len = Serial.parseInt();
        if (len == 0) len++;
        Serial.print(F("addr=0x"));
        Serial.print(addr,HEX);
        Serial.print(F(", len=0x"));
        Serial.print(len,HEX);
        Serial.println(F(":"));
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
        Serial.println(F("OK"));
        delete [] buf;
      }
      break;

    case 'S':
    case 's':
      {
        //R0,100
        Serial.print(F("New speed "));
        int speed = Serial.parseInt();
        if (speed < 1000) { speed = 1000; }
        Fram.setClock(speed);
        Serial.println(speed);
      }
      break;

    case 'W':
    case 'w':
      {
        //W0,test string[LF]
        //
        Serial.print(F("Write "));
        int addr = Serial.parseInt();
        Serial.print(F("0x"));
        Serial.print(addr,HEX);
        Serial.read();
        uint8_t *buf = new uint8_t[256];
        uint8_t len = Serial.readBytesUntil('\n',(char*)buf,256);
        Serial.print(F(",0x"));
        Serial.print(len,HEX);
        Serial.print(F(": "));
        Fram.write(addr,buf,len);
        Serial.println(F("OK"));
        delete [] buf;
      }
      break;

    case 'E':
    case 'e':
      {
        //e4096
        uint8_t *buf = new uint8_t[256];
        for(uint8_t x = 0; x<255; x++) { buf[x]= 0; }
        int len = Serial.parseInt();
        int addr = 0;
        int i,n;
        if(len==0) len = 255;
        n=len;
        Serial.println(F("Erasing..."));
        for(i = 0; i < 128; i++) 
        {
           len-=255;
           Fram.write(addr+(256*i),buf,(len<255? 255:len));
           if(len<255) break;
        }
        Serial.print(F("Erased ")); Serial.print(n); Serial.println(F(" bytes."));
        Serial.println(F("OK"));
        delete [] buf;
      }
      break;
    }
  } 
}


