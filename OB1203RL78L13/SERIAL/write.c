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

#include <LowLevelIOInterface.h>
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "r_cg_dmac.h"

#pragma module_name = "?__write"

static bool ready = true;

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
  while(false == ready);

  /* Stop the DMAC */
  R_DMAC0_Stop();
  
  /* Set the new address and count */
  R_DMAC0_SetAddressCount((uint16_t)buffer, size);

  /* Start the DMAC to send out the bytes */
  R_DMAC0_Start();
  ready = false;
  
  return size;
}
