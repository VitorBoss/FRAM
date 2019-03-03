//  Basic read/write functions for the MB85RS64A SPI FRAM chip
//  Copyright (C) 2017  Industruino <connect@industruino.com>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//  Developed by Claudio Indellicati <bitron.it@gmail.com>
//
//  Mod by Vitor_Boss on 01/2019
//    work with STM32
//    added option to use a secondary SPI
//    added software version of SPI with programmed speed

#include "Fram.h"

/*-----------------------------------------------------------------------------*/

FramClass::FramClass()
{
  clkPin = mosiPin = misoPin = -1;
  csPin = FRAM_DEFAULT_CS_PIN;
  spiSpeed = (uint32_t)F_CPU/FRAM_DEFAULT_CLOCK;
}

/*-----------------------------------------------------------------------------*/

FramClass::FramClass (uint8_t cp, SPIClass &_spi)
{
  clkPin = mosiPin = misoPin = -1;
  csPin = cp;
  spi = _spi;
  spiSpeed = (uint32_t)F_CPU/FRAM_DEFAULT_CLOCK;
  begin(csPin, spi);
}

/*-----------------------------------------------------------------------------*/

FramClass::FramClass (uint8_t cp, uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t clockspeed)
{
  csPin = cp;
  clkPin = clk;
  misoPin = miso;
  mosiPin = mosi;
  spiSpeed = (uint32_t)F_CPU/clockspeed;
  
  // Set CS pin HIGH and configure it as an output
  pinMode(csPin, OUTPUT);
  pinMode(clkPin, OUTPUT);
  pinMode(mosiPin, OUTPUT);
  pinMode(misoPin, INPUT_PULLUP);
  deassertCS;
}

/*-----------------------------------------------------------------------------*/

void FramClass::EnableWrite (boolean state)
{
  assertCS;
  if (state){ Send(FRAM_CMD_WREN); }
  else { Send(FRAM_CMD_WRDI); }
  deassertCS;
}

/*-----------------------------------------------------------------------------*/

void FramClass::setClock(uint32_t clockSpeed) {
  spiSpeed = (uint32_t)F_CPU/clockSpeed;
}


/*-----------------------------------------------------------------------------*/

void FramClass::begin (uint8_t cp, SPIClass &_spi)
{
  csPin = cp;
  spi = _spi;
  
  // Set CS pin HIGH and configure it as an output
  pinMode(csPin, OUTPUT);
  deassertCS;
  spi.begin();
#ifdef __SAM3X8E__
  spi.setClockDivider (9); // 9.3 MHz
#elif defined(STM32F2XX)
	spi.setClockDivider (SPI_CLOCK_DIV4); // SPI @ 15MHz
#elif defined(STM32F4) || defined(ARDUINO_ARCH_STM32)
	spi.setClockDivider (SPI_CLOCK_DIV8); // SPI @ 10MHz
#else
	spi.setClockDivider (SPI_CLOCK_DIV2); // 8 MHz
#endif
  spi.setDataMode(SPI_MODE0);
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::write (uint16_t addr, uint8_t data)
{
  EnableWrite(true);
  assertCS;
  Send(FRAM_CMD_WRITE);
  Send16(addr);
  Send(data);
  deassertCS;
  EnableWrite(false);

  return 0U;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::write (uint16_t addr, uint8_t *data, uint16_t count)
{
  if (addr + count > FRAM_SIZE)
    return 1U;

  if (count == 0U)
    return -1;

  EnableWrite(true);
  assertCS;
  Send(FRAM_CMD_WRITE);
  Send16(addr);
  for (uint16_t i = 0; i < count; ++i)
    Send(data[i]);
  deassertCS;
  EnableWrite(false);

  return 0U;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::read (uint16_t addr, uint8_t *dataBuffer, uint16_t count)
{
  if (addr + count > FRAM_SIZE)
    return 1U;

  if (count == 0U)
    return -1;

  assertCS;
  Send(FRAM_CMD_READ);
  Send16(addr);
  for (uint16_t i=0; i < count; ++i)
    dataBuffer[i] = Send(0x00);
  deassertCS;

  return 0U;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::read (uint16_t addr)
{
  uint8_t dataBuffer;

  assertCS;
  Send(FRAM_CMD_READ);
  Send16(addr);
  dataBuffer = Send(0x00);
  deassertCS;

  return dataBuffer;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::update (uint16_t addr, uint8_t data)
{
  if(read(addr) != data)
    write(addr, data);
  return 1U;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::readSR ()
{
  uint8_t dataBuffer;

  assertCS;
  Send(FRAM_CMD_RDSR);
  dataBuffer = Send(0x00);
  deassertCS;

  return dataBuffer;
}

/*-----------------------------------------------------------------------------*/

uint8_t FramClass::Send(uint8_t data) 
{
  if (clkPin == -1) { return spi.transfer(data); } 
  else
  {
    uint8_t reply = 0;
    for (int i=7; i>=0; i--)
    {
      reply <<= 1;
      setClockPin(LOW);
      digitalWrite(mosiPin, !!(data & (1<<i)));
      setClockPin(HIGH);
      reply |= digitalRead(misoPin);
    }
    return reply;
  }
}

/*-----------------------------------------------------------------------------*/

uint16_t FramClass::Send16(uint16_t data) 
{
  if (clkPin == -1) { return spi.transfer16(data); } 
  else 
  {
    uint16_t reply = 0;
    for (int i=15; i>=0; i--)
    {
      reply <<= 1;
      setClockPin(LOW);
      digitalWrite(mosiPin, !!(data & (1<<i)));
      setClockPin(HIGH);
      reply |= digitalRead(misoPin);
    }
    return reply;
  }
}

/*-----------------------------------------------------------------------------*/


//FramClass Fram;

