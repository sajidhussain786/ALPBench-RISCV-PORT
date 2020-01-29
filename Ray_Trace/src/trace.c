/* 
 * trace.c - This file contains the functions for firing primary rays
 *           and handling subsequent calculations
 *
 *   $Id: trace.c,v 1.116 2010/01/18 06:13:04 johns Exp $
 */

#include "machine.h"
#include "types.h"
#include "macros.h"
#include "vector.h"
#include "shade.h"
#include "camera.h"
#include "util.h"
#include "threads.h"
#include "parallel.h"
#include "intersect.h"
#include "ui.h"
#include "trace.h"

color trace(ray * primary) {
  if (primary->depth > 0) {
    intersect_objects(primary);
    return primary->scene->shader(primary);
  }

  /* if the ray is truncated, return the background texture as its color */
  return primary->scene->bgtexfunc(primary);
}

void * thread_trace(thr_parms * t) {
  unsigned long * local_mbox = NULL;
  scenedef * scene;
  color col;
  ray primary;
  int x, y, do_ui, hskip;
  int startx, stopx, xinc, starty, stopy, yinc, hsize, vres;

#if defined(_OPENMP)
#pragma omp parallel
{
#endif

  /*
   * Copy all of the frequently used parameters into local variables.
   * This seems to improve performance, especially on NUMA systems.
   */
  startx = t->startx;
  stopx  = t->stopx;
  xinc   = t->xinc;
 
  starty = t->starty;
  stopy  = t->stopy;
  yinc   = t->yinc;
 
  scene  = t->scene;
  hsize  = scene->hres*3;
  vres   = scene->vres;
  hskip  = xinc * 3;
  do_ui = (scene->mynode == 0 && t->tid == 0);

#if !defined(DISABLEMBOX)
   /* allocate mailbox array per thread... */
#if defined(_OPENMP)
  local_mbox = (unsigned long *)calloc(sizeof(unsigned long)*scene->numobjects, 1);
#else
  if (t->local_mbox == NULL)  
    local_mbox = (unsigned long *)calloc(sizeof(unsigned long)*scene->objgroup.numobjects, 1);
  else 
    local_mbox = t->local_mbox;
#endif
#else
  local_mbox = NULL; /* mailboxes are disabled */
#endif

#if defined(_OPENMP)
#pragma omp single
#endif
  /* 
   * If we are getting close to integer wraparound on the    
   * ray serial numbers, we need to re-clear the mailbox     
   * array(s).  Each thread maintains its own serial numbers 
   * so only those threads that are getting hit hard will    
   * need to re-clear their mailbox arrays.  In all likelihood,
   * the threads will tend to hit their counter limits at about
   * the same time though.
   * When compiled on platforms with a 64-bit long, this counter won't 
   * wraparound in _anyone's_ lifetime, so no need to even check....
   * On lesser-bit platforms, we're not quite so lucky, so we have to check.
   */
#if !defined(LP64)
  if (local_mbox != NULL) {
    if (t->serialno > (((unsigned long) 1) << ((sizeof(unsigned long) * 8) - 3))) {
      memset(local_mbox, 0, sizeof(unsigned long) * scene->objgroup.numobjects);
      t->serialno = 1;
    }
  }
#endif

  /* setup the thread-specific properties of the primary ray(s) */
  camray_init(scene, &primary, t->serialno, local_mbox, 
              rng_seed_from_tid_nodeid(t->tid, scene->mynode));


  /* 
   * Render the image in either RGB24 or RGB96F format
   */
  if (scene->imgbufformat == RT_IMAGE_BUFFER_RGB24) {
    /* 24-bit unsigned char RGB, RT_IMAGE_BUFFER_RGB24 */
    int addr, R,G,B;
    unsigned char *img = (unsigned char *) scene->img;

    /* copy the RNG state to cause increased coherence among */
    /* AO sample rays, significantly reducing granulation    */
    rng_frand_handle cachefrng = primary.frng;

#if defined(_OPENMP)
#pragma omp for schedule(runtime)
#endif
    for (y=starty; y<=stopy; y+=yinc) {
      addr = hsize * (y - 1) + (3 * (startx - 1));    /* scanline address */
      for (x=startx; x<=stopx; x+=xinc) {
        primary.frng = cachefrng; /* each pixel uses the same AO RNG seed */
        col=scene->camera.cam_ray(&primary, x, y);    /* generate ray */ 

        R = (int) (col.r * 255.0f); /* quantize float to integer */
        G = (int) (col.g * 255.0f); /* quantize float to integer */
        B = (int) (col.b * 255.0f); /* quantize float to integer */

        if (R > 255) R = 255;       /* clamp pixel value to range 0-255      */
        if (R < 0) R = 0;
        img[addr    ] = (byte) R;   /* Store final pixel to the image buffer */

        if (G > 255) G = 255;       /* clamp pixel value to range 0-255      */
        if (G < 0) G = 0;
        img[addr + 1] = (byte) G;   /* Store final pixel to the image buffer */

        if (B > 255) B = 255;       /* clamp pixel value to range 0-255      */
        if (B < 0) B = 0;
        img[addr + 2] = (byte) B;   /* Store final pixel to the image buffer */

        addr += hskip;
      } /* end of x-loop */

      if (do_ui && !((y-1) % 16)) {
        rt_ui_progress((100 * y) / vres);  /* call progress meter callback */
      } 

#ifdef MPI
      if (scene->nodes > 1) {
        rt_thread_barrier(t->runbar, 1);
        if (t->tid == 0) {
          rt_sendrecvscanline(scene->parbuf); /* only thread 0 can use MPI */ 
        }
      }
#endif

    }        /* end y-loop */
  } else {   /* end of RGB24 loop */
    /* 96-bit float RGB, RT_IMAGE_BUFFER_RGB96F */
    int addr;
    float *img = (float *) scene->img;

    /* copy the RNG state to cause increased coherence among */
    /* AO sample rays, significantly reducing granulation    */
    rng_frand_handle cachefrng = primary.frng;

#if defined(_OPENMP)
#pragma omp for schedule(runtime)
#endif
    for (y=starty; y<=stopy; y+=yinc) {
      addr = hsize * (y - 1) + (3 * (startx - 1));    /* scanline address */
      for (x=startx; x<=stopx; x+=xinc) {
        primary.frng = cachefrng; /* each pixel uses the same AO RNG seed */
        col=scene->camera.cam_ray(&primary, x, y);    /* generate ray */ 
        img[addr    ] = col.r;   /* Store final pixel to the image buffer */
        img[addr + 1] = col.g;   /* Store final pixel to the image buffer */
        img[addr + 2] = col.b;   /* Store final pixel to the image buffer */
        addr += hskip;
      } /* end of x-loop */

      if (do_ui && !((y-1) % 16)) {
        rt_ui_progress((100 * y) / vres);  /* call progress meter callback */
      } 

#ifdef MPI
      if (scene->nodes > 1) {
        rt_thread_barrier(t->runbar, 1);
        if (t->tid == 0) {
          rt_sendrecvscanline(scene->parbuf); /* only thread 0 can use MPI */ 
        }
      }
#endif

    }        /* end y-loop */
  }          /* end of RGB96F loop */

  /* 
   * Image has been rendered into the buffer in the appropriate pixel format
   */

  t->serialno = primary.serial + 1;

#if defined(_OPENMP)
  if (local_mbox != NULL)
    free(local_mbox);
#else
  if (t->local_mbox == NULL) {
    if (local_mbox != NULL)
      free(local_mbox);
  }
#endif

  if (scene->nodes == 1)
    rt_thread_barrier(t->runbar, 1);

#if defined(_OPENMP)
  }
#endif

  return(NULL);  
}

