/*
 * tachyon.h - The declarations and prototypes needed so that 3rd party     
 *   driver code can run the raytracer.  Third party driver code should       
 *   only use the functions in this header file to interface with the 
 *   rendering engine.                                            
 *
 * $Id: tachyon.h,v 1.87 2009/05/15 20:25:01 johns Exp $
 *
 */

#if !defined(TACHYON_H)
#define TACHYON_H 1

#ifdef  __cplusplus
extern "C" {
#endif

#include "rtcommon.h" /* defintions common to all interfaces */

/********************************************/
/* Types defined for use with the API calls */
/********************************************/

typedef void * SceneHandle;

typedef struct {
   apiflt x;
   apiflt y;
   apiflt z;
} apivector;

typedef struct {
   float r;
   float g;
   float b;
} apicolor;

typedef struct {
  int texturefunc; /* which texture function to use */
  apicolor col;    /* base object color */
  int shadowcast;  /* does the object cast a shadow */
  apiflt ambient;  /* ambient lighting */
  apiflt diffuse;  /* diffuse reflection */
  apiflt specular; /* specular reflection */
  apiflt opacity;  /* how opaque the object is */ 
  apivector ctr;   /* origin of texture */
  apivector rot;   /* rotation of texture around origin */
  apivector scale; /* scale of texture in x,y,z */ 
  apivector uaxs;  /* planar map u axis */
  apivector vaxs;  /* planar map v axis */
  apivector waxs;  /* volume map W axis */
  char imap[96];   /* name of image map */ 
} apitexture;

/********************************************/
/* Functions implemented to provide the API */
/********************************************/

apivector rt_vector(apiflt x, apiflt y, apiflt z); /* helper to make vectors */
apicolor  rt_color(apiflt r, apiflt g, apiflt b);  /* helper to make colors */

void rt_set_ui_message(void (* func) (int, char *)); 
void rt_set_ui_progress(void (* func) (int));

/*
 * rt_initialize(&argc, &argv)
 *
 * takes pointer to argument count, and pointer to argument array
 * 1. resets and initializes the raytracing system
 * 2. initializes internal parallel processing facilities, and tests
 *    inter-node connectivity.
 * 3. deallocates previously allocated internal data structures
 * 4. returns the id of this computational node on success, -1 on failure.
 */ 
int rt_initialize(int *, char ***); 


/*
 * rt_finalize()
 *
 * shutdown the ray tracing library for good, at final use before
 * program termination.  The ray tracer may not be used after rt_finalize
 * has been called.
 */ 
void rt_finalize(void); 

/*
 * rt_newscene()
 *
 * Allocates, initializes, and returns a handle for a new scene.  
 */
SceneHandle rt_newscene(void); 


/* 
 * rt_deletescene(SceneHandle)
 *
 * Destroys / deallocates the specified scene.
 */
void rt_deletescene(SceneHandle);

/*
 * rt_renderscene(SceneHandle)
 *
 * Renders the current scene.
 */
void rt_renderscene(SceneHandle);

/*
 * rt_outputfile(SceneHandle, outname);
 * 
 * Sets the output filename of the specified scene.
 */
void rt_outputfile(SceneHandle, const char * outname); 

/*
 * rt_outputformat(SceneHandle, format);
 *
 * Sets the format of the output image(s)
 */
void rt_outputformat(SceneHandle, int format);

/*
 * rt_resolution(SceneHandle, hres, vres)
 * rt_get_resolution(SceneHandle, &hres, &vres)
 *
 * Sets the horizontal and vertical resolution (in pixels)
 * for the specified scene.
 */
void rt_resolution(SceneHandle, int hres, int vres);
void rt_get_resolution(SceneHandle, int *hres, int *vres);

/* 
 * rt_crop_output(SceneHandle, hres, vres, lx, ly)
 * 
 * Crop the output image to the specified size
 */
void rt_crop_output(SceneHandle, int hres, int vres, int lx, int ly);
void rt_crop_disable(SceneHandle);

/*
 * rt_aa_maxsamples(SceneHandle, maxsamples)
 * 
 * Sets the maximum number of supersamples to take for any pixel.
 */
void rt_aa_maxsamples(SceneHandle, int maxsamples);

/*
 * rt_verbose(SceneHandle, onoff);
 *
 * Enables or Disables verbose messages from the ray tracing library
 * during rendering. (a zero value means off, non-zero means on)
 */
void rt_verbose(SceneHandle, int v);

/*
 * rt_normal_fixup_mode(SceneHandle, mode)
 *
 * Set the surface normal / winding order fixup mode to use
 * when generating triangles using interpolated surface normals.
 */
void rt_normal_fixup_mode(SceneHandle, int mode);

/*
 * rt_image_clamp(SceneHandle)
 * clamp pixel values to the range [0 1)
 */
void rt_image_clamp(SceneHandle voidscene);

/*
 * rt_image_normalize(SceneHandle)
 * renormalize pixel values to the range [0 1)
 */
void rt_image_normalize(SceneHandle voidscene);

/*
 * rt_image_gamma(SceneHandle, gamma)
 * apply gamma correction to the pixel values after normalization
 */
void rt_image_gamma(SceneHandle voidscene, float gamma);

/*
 * rt_rawimage_rgb24(SceneHandle, unsigned char *rawimage);
 *
 * Have the ray tracer save the output image in the specified
 * memory area, in raw 24-bit, packed, pixel interleaved, unsigned
 * RGB bytes.  The caller is responsible for making sure that there
 * is enough space in the memory area for the entire image.
 */
void rt_rawimage_rgb24(SceneHandle, unsigned char *rawimage);

/*
 * rt_rawimage_rgb96f(SceneHandle, float *rawimage);
 *
 * Have the ray tracer save the output image in the specified
 * memory area, in raw 96-bit, packed, pixel interleaved, 32-bit float
 * RGB bytes.  The caller is responsible for making sure that there
 * is enough space in the memory area for the entire image.
 */
void rt_rawimage_rgb96f(SceneHandle, float *rawimage);

/*
 * rt_set_numthreads(SceneHandle, int) 
 *
 * Explicitly sets the number of threads the ray tracer will use
 */
void rt_set_numthreads(SceneHandle, int);

/*
 * rt_background(SceneHandle, apicolor);
 *
 * Sets the background color of the specified scene.
 */
void rt_background(SceneHandle, apicolor);

/*
 * rt_background_sky_sphere(SceneHandle, up direction, 
 *                          max value, min value, max color, min color);
 *
 * Set parameters for sky sphere background texturing
 */
void rt_background_sky_sphere(SceneHandle, apivector, apiflt, apiflt,
                              apicolor, apicolor);

/* 
 * rt_background_mode(SceneHandle, mode flag);
 *
 * Set the background texturing mode to use
 */
void rt_background_mode(SceneHandle, int);


/*
 * rt_fog_rendering_mode(SceneHandle, mode);
 */
void rt_fog_rendering_mode(SceneHandle, int);

/*
 * rt_fog_mode(SceneHandle, mode);
 */
void rt_fog_mode(SceneHandle, int);

/*
 * rt_fog_parms(SceneHandle, color, start, end, density);
 */
void rt_fog_parms(SceneHandle, apicolor, apiflt, apiflt, apiflt);


/*
 * rt_trans_mode(SceneHandle, mode);
 */
void rt_trans_mode(SceneHandle, int);



/*
 * rt_boundmode(SceneHandle, int)
 * 
 * Enables/Disables automatic generation and use of ray tracing acceleration
 * data structures. 
 */
void rt_boundmode(SceneHandle, int);


/* 
 * rt_boundthresh(SceneHandle, int)
 * 
 * Sets the threshold to be used when automatic generation of ray tracing
 * acceleration structures is to be used.  The threshold represents the 
 * minimum number of objects which must be present in an area of space 
 * before an automatic acceleration system will consider optimizing the
 * objects using spatial subdivision or automatic bounds generation methods.
 */
void rt_boundthresh(SceneHandle, int);

/* 
 * rt_shadermode()
 *
 * Sets the shading mode for the specified scene. 
 *
 * Modes are sorted from lowest quality (and fastest execution)
 * to highest quality (and slowest execution)
 */
void rt_shadermode(SceneHandle voidscene, int mode);

/*
 * rt_phong_shader()
 *
 * Set the equation used for rendering specular highlights
 *
 */
void rt_phong_shader(SceneHandle voidscene, int mode);
 

/*
 * rt_camera_setup(scene, zoom, aspect, alias, maxdepth, ctr, viewdir, updir)
 *
 * NOTE: This API should be deprecated, but a suitable replacement has not 
 *       been written yet.
 */
void rt_camera_setup(SceneHandle, apiflt, apiflt, int, int,
	apivector, apivector,  apivector);

/*
 * rt_camera_projection(scene, mode)
 */
void rt_camera_projection(SceneHandle, int);

/*
 * set depth-of-field options
 */
void rt_camera_dof(SceneHandle voidscene, flt focallength, flt aperture);

/* 
 * rt_camera_position(scene, center, viewdir, updir)
 */ 
void rt_camera_position(SceneHandle, apivector, apivector, apivector);

/* 
 * rt_get_camera_position(scene, center, viewdir, updir, rightdir)
 */ 
void rt_get_camera_position(SceneHandle, apivector *, apivector *, apivector *, apivector *);

/*
 * rt_camera_frustum(scene, left, right, bottom, top)
 */
void rt_camera_frustum(SceneHandle, flt, flt, flt, flt);

/*
 * rt_texture(SceneHandle, texture *)
 *
 * translates a texture definition into the internal format used
 * by the ray tracing system, and returns an opaque pointer to the
 * internally used structure, which should be passed to object creation
 * routines.
 *
 * NOTE: This API should be deprecated, but a suitable replacement has not 
 *       been written yet.
 */
void * rt_texture(SceneHandle, apitexture *);

/*
 * rt_define_image(name, xsize, ysize, zsize, packed 24-bit data)
 * 
 * defines a named image without external file references
 */
void rt_define_image(const char *, int, int, int, unsigned char *);

/* 
 * Do not use this unless you know what you're doing, this is a 
 * short-term workaround until new object types have been created.
 */
void * rt_texture_copy_standard(SceneHandle, void *oldtex);
void * rt_texture_copy_vcstri(SceneHandle, void *oldtex);

/* rescale all light sources in the scene by factor lightscale */
void rt_rescale_lights(SceneHandle, apiflt lightscale);

void * rt_light(SceneHandle, void * , apivector, apiflt);     
  /* add a point light */
  /* parms: texture, center, radius */ 

void * rt_directional_light(SceneHandle, void * , apivector);     
  /* add a directional light */
  /* parms: texture, direction */ 

void * rt_spotlight(SceneHandle, void * , apivector, apiflt, apivector, apiflt, apiflt);     
  /* add a spotlightlight */
  /* parms: texture, center, radius, direction, falloffstart, falloffend */ 

void rt_light_attenuation(void *, apiflt, apiflt, apiflt);
  /* parms: Light, Constant factor, Linear factor, Quadratic factor */

/*
 * Ambient occlusion shading, with monte carlo sampling of "sky" light
 */
void rt_ambient_occlusion(void *, int, apicolor);

void rt_sphere(SceneHandle, void *, apivector, apiflt);    /* add a sphere */
  /* sphere parms: texture, center, radius */

void rt_scalarvol(SceneHandle, void *, apivector, apivector,
		 int, int, int, const char *, void *); 

void rt_extvol(SceneHandle, void *, apivector, apivector, int, apiflt (* evaluator)(apiflt, apiflt, apiflt)); 

void rt_box(SceneHandle, void *, apivector, apivector);  
  /* box parms: texture, min, max */

void rt_plane(SceneHandle, void *, apivector, apivector);  
  /* plane parms: texture, center, normal */

void rt_ring(SceneHandle, void *, apivector, apivector, apiflt, apiflt); 
  /* ring parms: texture, center, normal, inner, outer */

void rt_tri(SceneHandle, void *, apivector, apivector, apivector);  
  /* tri parms: texture, vertex 0, vertex 1, vertex 2 */

void rt_stri(SceneHandle, void *, apivector, apivector, apivector, 
			apivector, apivector, apivector); 
 /* stri parms: texture, vertex 0, vertex 1, vertex 2, norm 0, norm 1, norm 2 */

void rt_vcstri(SceneHandle, void *, apivector, apivector, apivector, 
			apivector, apivector, apivector,
			apicolor, apicolor, apicolor); 
 /* vcstri parms: texture, vertex 0, vertex 1, vertex 2,            */
 /*               norm 0, norm 1, norm 2, color 0, color 1, color 2 */

void rt_tristripscnv3fv(SceneHandle scene, void * tex,
                        int numverts, const float * cnv,
                        int numstrips, int *vertsperstrip, int *facets);

void rt_heightfield(SceneHandle, void *, apivector, int, int, apiflt *, apiflt, apiflt);
  /* field parms: texture, center, m, n, field, wx, wy */

void rt_landscape(SceneHandle, void *, int, int, apivector,  apiflt, apiflt);

void rt_quadsphere(SceneHandle, void *, apivector, apiflt); /* add quadric sphere */
  /* sphere parms: texture, center, radius */

void rt_cylinder(SceneHandle, void *, apivector, apivector, apiflt);

void rt_fcylinder(SceneHandle, void *, apivector, apivector, apiflt);

void rt_polycylinder(SceneHandle, void *, apivector *, int, apiflt);

/* new texture handling routines */
void rt_tex_phong(void * voidtex, apiflt phong, apiflt phongexp, int type); 

void rt_tex_outline(void * voidtex, apiflt outline, apiflt outlinewidth); 

/* set active clipping plane group */
void rt_clip_fv(SceneHandle, int numplanes, float * planes);
void rt_clip_dv(SceneHandle, int numplanes, double * planes);
void rt_clip_off(SceneHandle);

#ifdef  __cplusplus
}
#endif
#endif
