//
//  SensorSerialProtocol.h
//  Komakusa
//
//  Created by arms22 on 2012/08/20.
//  Copyright (c) 2012 arms22. All rights reserved.
//

#ifndef SENSOR_SERIAL_PROTOCOL
#define SENSOR_SERIAL_PROTOCOL

#define SSP_READ      (0x5a)
#define SSP_DATA_LEN  (5)
/*
 SSP_READ->
 <- TEMP_H
 <- TEMP_L
 <- HUMI_H
 <- HUMI_L
 <- CRC8
 */

static uint8_t crc8(const uint8_t *buffer, size_t size)
{
  const uint8_t POLY = 0x31;   // Polynomial: x**8 + x**5 + x**4 + 1
  uint8_t crc = 0x00;
  while(size){
    crc ^= *buffer++;
    for(int i = 0 ; i < 8; i++){
      if(crc & 0x80){
        crc <<= 1; 
        crc ^= POLY;
      }
      else{
        crc <<= 1;
      }
    }
    size--;
  }
  return crc;
}

#endif

