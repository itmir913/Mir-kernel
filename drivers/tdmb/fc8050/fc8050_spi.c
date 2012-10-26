/*****************************************************************************
 Copyright(c) 2009 FCI Inc. All Rights Reserved
 
 File name : fc8050_spi.c
 
 Description : fc8050 host interface
 
 History : 
 ----------------------------------------------------------------------
 2009/08/29   jason   initial
*******************************************************************************/
#include "fci_types.h"
#include "fc8050_regs.h"
//#include "rex.h"

#define HPIC_READ     0x01 // read command
#define HPIC_WRITE    0x02 // write command
#define HPIC_AINC     0x04 // address increment
#define HPIC_BMODE    0x00 // byte mode
#define HPIC_WMODE    0x10 // word mode
#define HPIC_LMODE    0x20 // long mode
#define HPIC_ENDIAN   0x00 // little endian
#define HPIC_CLEAR    0x80 // currently not used

#define CHIPID 0
#if (CHIPID == 0)
#define SPI_CMD_WRITE                           0x0
#define SPI_CMD_READ                            0x1
#define SPI_CMD_BURST_WRITE                     0x2
#define SPI_CMD_BURST_READ                      0x3
#else
#define SPI_CMD_WRITE                           0x4
#define SPI_CMD_READ                            0x5
#define SPI_CMD_BURST_WRITE                     0x6
#define SPI_CMD_BURST_READ                      0x7
#endif 

//LOCAL rex_crit_sect_type  bbm_spi_crit_sect;
static int spi_bulkread(HANDLE hDevice, u8 addr, u8 *data, u16 length)
{
  return BBM_OK;
}

static int spi_bulkwrite(HANDLE hDevice, u8 addr, u8* data, u16 length)
{
  return BBM_OK;
}

static int spi_dataread(HANDLE hDevice, u8 addr, u8* data, u16 length)
{
  return spi_bulkread(hDevice, addr, data, length);
}

int fc8050_spi_init(HANDLE hDevice, u16 param1, u16 param2)
{
  return BBM_OK;
}

int fc8050_spi_byteread(HANDLE hDevice, u16 addr, u8 *data)
{
  int res;
  u8 command = HPIC_READ | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkread(hDevice, BBM_DATA_REG, data, 1);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_wordread(HANDLE hDevice, u16 addr, u16 *data)
{
  int res;
  u8 command = HPIC_READ | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  if(BBM_SCI_DATA <= addr && BBM_SCI_SYNCRX >= addr)
    command = HPIC_READ | HPIC_WMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkread(hDevice, BBM_DATA_REG, (u8*)data, 2);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_longread(HANDLE hDevice, u16 addr, u32 *data)
{
  int res;
  u8 command = HPIC_READ | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkread(hDevice, BBM_DATA_REG, (u8*)data, 4);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_bulkread(HANDLE hDevice, u16 addr, u8 *data, u16 length)
{
  int res;
  u8 command = HPIC_READ | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkread(hDevice, BBM_DATA_REG, data, length);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_bytewrite(HANDLE hDevice, u16 addr, u8 data)
{
  int res;
  u8 command = HPIC_WRITE | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkwrite(hDevice, BBM_DATA_REG, (u8*)&data, 1);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_wordwrite(HANDLE hDevice, u16 addr, u16 data)
{
  int res;
  u8 command = HPIC_WRITE | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  if(BBM_SCI_DATA <= addr && BBM_SCI_SYNCRX >= addr)
    command = HPIC_WRITE | HPIC_WMODE | HPIC_ENDIAN;


  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkwrite(hDevice, BBM_DATA_REG, (u8*)&data, 2);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_longwrite(HANDLE hDevice, u16 addr, u32 data)
{
  int res;
  u8 command = HPIC_WRITE | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkwrite(hDevice, BBM_DATA_REG, (u8*)&data, 4);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_bulkwrite(HANDLE hDevice, u16 addr, u8* data, u16 length)
{
  int res;
  u8 command = HPIC_WRITE | HPIC_AINC | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_bulkwrite(hDevice, BBM_DATA_REG, data, length);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_dataread(HANDLE hDevice, u16 addr, u8* data, u16 length)
{
  int res;
  u8 command = HPIC_READ | HPIC_BMODE | HPIC_ENDIAN;

  //rex_enter_crit_sect(&bbm_spi_crit_sect);

  res  = spi_bulkwrite(hDevice, BBM_COMMAND_REG, &command, 1);
  res |= spi_bulkwrite(hDevice, BBM_ADDRESS_REG, (u8*)&addr, 2);
  res |= spi_dataread(hDevice, BBM_DATA_REG, data, length);

  //rex_leave_crit_sect(&bbm_spi_crit_sect);

  return res;
}

int fc8050_spi_deinit(HANDLE hDevice)
{
  return BBM_OK;
}
