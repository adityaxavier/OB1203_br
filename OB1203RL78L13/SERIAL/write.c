/*******************
 *
 * Copyright 1998-2017 IAR Systems AB.  
 *
 * This is a template implementation of the "__write" function used by
 * the standard library.  Replace it with a system-specific
 * implementation.
 *
 * The "__write" function should output "size" number of bytes from
 * "buffer" in some application-specific way.  It should return the
 * number of characters written, or _LLIO_ERROR on failure.
 *
 * If "buffer" is zero then __write should perform flushing of
 * internal buffers, if any.  In this case "handle" can be -1 to
 * indicate that all handles should be flushed.
 *
 * The template implementation below assumes that the application
 * provides the function "MyLowLevelPutchar".  It should return the
 * character written, or -1 on failure.
 *
 ********************/
#include <string.h>
#include <LowLevelIOInterface.h>
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "r_cg_dmac.h"
#include "r_cg_sau.h"

#pragma module_name = "?__write"
   
#define TSF_BITMASK     (0x0040)
#define DMA_BYTE_COUNT  (_0040_DMA0_BYTE_COUNT)

static volatile bool ready = true;

/*
 * This function is invoked by the DMAC Interrupt handler when all bytes
 * are transferred.
 */
void write_done(void)
{
  ready = true;
}

/*
 * If the __write implementation uses internal buffering, uncomment
 * the following line to ensure that we are called with "buffer" as 0
 * (i.e. flush) when the application terminates.
 */

size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  extern uint8_t g_Dmac0[];
  uint8_t sending = 0;
  if (buffer == 0)
  {
    /*
     * This means that we should flush internal buffers.  Since we
     * don't we just return.  (Remember, "handle" == -1 means that all
     * handles should be flushed.)
     */
    return 0;
  }

  /* This template only writes to "standard out" and "standard err",
   * for all other file handles it returns failure. */
  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
  {
    return _LLIO_ERROR;
  }
  
  /* Wait until any previous transfers are completed */
  while((false == ready) && ((SSR10 & TSF_BITMASK) > 0));

  /* Enable the DMA channel */
  if(1 != DEN0)
  {
    DEN0 = 1;
  }
  /* Set the new address and count */
  DRA0 = (uint16_t)&g_Dmac0[0];
  if((size - 1) > DMA_BYTE_COUNT)
  {
    memcpy(&g_Dmac0[0],&buffer[1], DMA_BYTE_COUNT);
    DBC0 = sending = DMA_BYTE_COUNT;
    
  }
  else
  {
    if(size > 1)
    {
      memcpy(&g_Dmac0[0],&buffer[1], size-1);
      DBC0 = size-1;
      sending = size;
    }
  }
  
  /* Start the DMAC operation */
  R_DMAC0_Start();
  
  /* Write the first byte to the DMAC controller */
  TXD2 = buffer[0];
  ready = false;
  
  return sending;
}
