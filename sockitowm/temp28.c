//---------------------------------------------------------------------------
// Copyright (C) 2000 Dallas Semiconductor Corporation, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Dallas Semiconductor
// shall not be used except as stated in the Dallas Semiconductor
// Branding Policy.
// ---------------------------------------------------------------------------
//
// temp28.C - Module to read the DS18B20 - temperature measurement.
//
// ---------------------------------------------------------------------------
//
//
#include "ownet.h"
#include "temp28.h"

//----------------------------------------------------------------------
// Read the temperature of a DS18B20 (family code 0x28)
//
// 'portnum' - number 0 to MAX_PORTNUM-1. This number was provided to
// OpenCOM to indicate the port number.
// 'SerialNum' - Serial Number of DS18B20 to read temperature from
// 'Temp ' - pointer to variable where that temperature will be
// returned
//
// Returns: TRUE(1) temperature has been read and verified
// FALSE(0) could not read the temperature, perhaps device is not
// in contact
//
uint8_t ReadTemperature28(uint8_t portnum, uint8_t *SerialNum, int16_t *Temp)
{
  uint8_t rt=FALSE;
  uint8_t send_block[30],lastcrc8 = 0;
  int send_cnt, tsht, i;
  static int power = -1;
  static uint8_t toggle = 0;

  // set the device serial number to the counter device
  owSerialNum(portnum,SerialNum,FALSE);

  //we need to call this only once at the beginning
  if(power==-1)
  {
    // check if the chip is connected to VDD
    if (owAccess(portnum))
    {
      owWriteByte(portnum,0xB4);
      power = owReadByte(portnum);
    }
  }

  if(toggle) 
  {
    if( owAccess(portnum))
    {
      // turn off the 1-Wire Net strong pull-up
      if (power) {
        if (owLevel(portnum,MODE_NORMAL) != MODE_NORMAL)
          return FALSE;
      }

      while(!owReadByte(portnum)){} //wait until conversion is done
      // access the device
      if (owAccess(portnum))
      {
        // create a block to send that reads the temperature
        // read scratchpad command
        send_cnt = 0;
        send_block[send_cnt++] = 0xBE;
        // now add the read bytes for data bytes and crc8
        for (i = 0; i < 9; i++)
          send_block[send_cnt++] = 0xFF;

        // now send the block
        if (owBlock(portnum,FALSE,send_block,send_cnt))
        {
          // initialize the CRC8
          setcrc8(portnum,0);
          // perform the CRC8 on the last 8 bytes of packet
          for (i = send_cnt - 9; i < send_cnt; i++)
            lastcrc8 = docrc8(portnum,send_block[i]);

          // verify CRC8 is correct
          if (lastcrc8 == 0x00)
          {
            // calculate the high-res temperature
            tsht = send_block[2] << 8;
            tsht = tsht | send_block[1];
            if (tsht & 0x00001000)
              tsht = tsht | 0xffff0000;
            *Temp = (int16_t) tsht;
            // success
            rt = TRUE;
            //break;
          }
        }
      }
    }
  }

  // access the device
  if (owAccess(portnum))
  {
    // send the convert command and if nesessary start power delivery
    if (power) {
      if (!owWriteBytePower(portnum,0x44))
        return FALSE;
    } else {
      if (!owWriteByte(portnum,0x44))
        return FALSE;
    }
    toggle=1;
  }

  // return the result flag rt
  return rt;
}
