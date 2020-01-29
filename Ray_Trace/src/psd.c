/*
 *  psd.c - This file deals with Photoshop format image files (reading/writing)
 *
 *  $Id: psd.c,v 1.2 2007/02/13 04:38:48 johns Exp $
 */ 

#include <stdio.h>
#include "machine.h"
#include "types.h"
#include "util.h"
#include "imageio.h" /* error codes etc */
#include "psd.h"

int writepsd48(char *name, int xres, int yres, unsigned char *imgdata) {
  FILE * ofp;
  int y, p;
  char width[4];
  char height[4];
  const char *sig  = "8BPS";                      /* signature           */
  const char ver[] = { 0, 1, 0, 0, 0, 0, 0, 0 };  /* version info        */
  const char chn[] = { 0, 3 };                    /* 3 channels          */
  const char mod[] = { 0, 16, 0, 3 };             /* 16-bit color, 3=rgb */
  const char hdr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 
  ofp=fopen(name, "wb");
  if (ofp==NULL) {
    return IMAGEBADFILE;
  }

  width[0] = (xres >> 24) & 0xff;
  width[1] = (xres >> 16) & 0xff;
  width[2] = (xres >>  8) & 0xff;
  width[3] = xres & 0xff;

  height[0] = (yres >> 24) & 0xff;
  height[1] = (yres >> 16) & 0xff;
  height[2] = (yres >>  8) & 0xff;
  height[3] = yres & 0xff;

  fwrite(sig, 4, 1, ofp);
  fwrite(ver, 8, 1, ofp);
  fwrite(chn, 2, 1, ofp);
  fwrite(height, 4, 1, ofp);
  fwrite(width, 4, 1, ofp);
  fwrite(mod, 4, 1, ofp);
  fwrite(hdr, 14, 1, ofp);

  for (p=0; p<3; p++) {
    int paddr = xres * yres * 2 * p;
    for (y=0; y<yres; y++) {
      fwrite(&imgdata[paddr + (yres - y - 1)*xres*2], 1, 2*xres, ofp);
    }
  }

  fclose(ofp);

  return IMAGENOERR;
}


