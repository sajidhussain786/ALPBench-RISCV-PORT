/* 
 * light.h - this file includes declarations and defines for light sources.
 *
 *  $Id: light.h,v 1.15 2007/02/07 09:20:13 johns Exp $
 */

#include "shade.h"

typedef struct light_t {
  RT_OBJECT_HEAD
  flt (* shade_diffuse)(struct light_t *, shadedata *);   /* diffuse shading function       */
} light;

typedef struct point_light_t {
  RT_OBJECT_HEAD
  flt (* shade_diffuse)(struct point_light_t *, shadedata *);   /* diffuse shading function       */
  vector ctr;
  flt rad;
  flt (* attenuationfunc)(void *, flt);
  flt Kc;
  flt Kl; 
  flt Kq;
  flt (* spotfunc)(void *, vector *);
  vector spotdir;
  flt fallstart;
  flt fallend;
} point_light; 

typedef struct directional_light_t {
  RT_OBJECT_HEAD
  flt (* shade_diffuse)(struct directional_light_t *, shadedata *);   /* diffuse shading function       */
  vector dir;
} directional_light; 


void free_light_special(void *voidlight);

directional_light * newdirectionallight(void *, vector);

point_light * newpointlight(void *, vector, flt);
point_light * newspotlight(void * tex, vector ctr, flt rad, vector dir,
                           flt fallstart, flt fallend);

point_light * newlight(void *, vector, flt);
point_light * newspotlight(void * tex, vector ctr, flt rad, vector dir,
                           flt fallstart, flt fallend);

void light_set_attenuation(point_light * li, flt Kc, flt Kl, flt Kq);


#ifdef LIGHT_PRIVATE
static int light_bbox(void * obj, vector * min, vector * max);
static void light_intersect(const point_light *, ray *);
static void light_normal(const point_light *, const vector *, const ray *, vector *);
static flt light_no_attenuation(void * vli, flt Llen);
static flt light_complex_attenuation(void * vli, flt Llen);
static flt light_no_falloff(void * vli, vector * L);
static flt light_spotlight_falloff(void * vli, vector * L);
static flt point_light_shade_diffuse(point_light * li, shadedata *);
static flt simple_point_light_shade_diffuse(point_light * li, shadedata *);
static flt directional_light_shade_diffuse(directional_light * li, shadedata *);
#endif

