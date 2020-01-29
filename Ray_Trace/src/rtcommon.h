/*
 * rtcommon.h - This file contains common defines typedefs used by various
 *   parts of the raytracer and it interfaces
 *
 * $Id: rtcommon.h,v 1.34 2009/11/05 21:15:52 johns Exp $
 *
 */ 

#include "util.h"    /* rt_timer_xxx() and rt_rand() */
#include "hash.h"    /* rt_hash_xxx() */

/*
 * Tachyon version strings for feature detection and compatibility testing.
 */
#define TACHYON_VERSION_STRING      "0.98.9"
#define TACHYON_MAJOR_VERSION       0
#define TACHYON_MINOR_VERSION       98
#define TACHYON_PATCH_VERSION       9

/* 
 * Build the ray tracer and its interfaces using either doubles or floats
 * based on compile time defition of USESINGLEFLT
 */
#ifdef USESINGLEFLT
typedef float flt;   /* generic floating point number, using float  */
typedef flt apiflt;  /* generic floating point number, using float  */
#else
typedef double flt;  /* generic floating point number, using double */
typedef flt apiflt;  /* generic floating point number, using double */
#endif


/*
 * Parameter values for rt_boundmode()
 */
#define RT_BOUNDING_DISABLED 0  /* spatial subdivision/bounding disabled */
#define RT_BOUNDING_ENABLED  1  /* spatial subdivision/bounding enabled  */


/*
 * Parameter values for rt_camera_projection()
 */
#define RT_PROJECTION_PERSPECTIVE      0
#define RT_PROJECTION_ORTHOGRAPHIC     1
#define RT_PROJECTION_PERSPECTIVE_DOF  2
#define RT_PROJECTION_FISHEYE          3

/* 
 * Fog modes
 */
#define RT_FOG_NORMAL     0 /* radial fog                             */
#define RT_FOG_VMD        1 /* planar OpenGL-like fog                 */

#define RT_FOG_NONE       0 /* no fog                                 */ 
#define RT_FOG_LINEAR     1 /* linear fog                             */
#define RT_FOG_EXP        2 /* exponential fog                        */
#define RT_FOG_EXP2       3 /* exponential-squared fog                */

/* 
 * Transparency modes
 */
#define RT_TRANS_ORIG     0 /* original transparency mode               */
#define RT_TRANS_VMD      1 /* mult shaded color by opacity, for VMD    */

/* 
 * Surface normal / winding order fixup mode
 * used for triangles with interpolate normals
 */
#define RT_NORMAL_FIXUP_OFF    0 /* surface normals and winding order agree  */
#define RT_NORMAL_FIXUP_FLIP   1 /* flip normals to agree with winding order */
#define RT_NORMAL_FIXUP_GUESS  2 /* random normal/winding order, guess       */

/*
 * Shader Modes for rt_shadermode()
 *
 * These are sorted from lowest quality (and fastest execution)
 * to highest quality (and slowest execution)
 */
#define RT_SHADER_AUTO    0  /* Automatically determine shader needed */
#define RT_SHADER_LOWEST  1  /* lowest quality shading available      */
#define RT_SHADER_LOW     2  /* low quality shading                   */
#define RT_SHADER_MEDIUM  3  /* Medium quality shading                */
#define RT_SHADER_HIGH    4  /* High quality shading                  */
#define RT_SHADER_FULL    5  /* Highest quality shading available     */

/*
 * Shader modes for rt_phong_shader()
 */
#define RT_SHADER_NULL_PHONG 0 /* Disable phong contributions               */
#define RT_SHADER_BLINN_FAST 1 /* Fast version of Blinn's equation          */
#define RT_SHADER_BLINN      2 /* Blinn's specular highlights, as in OpenGL */
#define RT_SHADER_PHONG      3 /* Phong specular highlights                 */

/*
 * Shader capability flags - sorted by relative execution cost.
 *
 * Used to automatically setup the fastest shader that supports
 * all of the capabilities used in a given scene.
 * Ideally, we use the shader that just has the features we need,
 * and nothing more, but its impractical to have that many seperate
 * shaders, each optimized for an exact set of features, but we
 * do the best we can with a reasonable amount of code.
 */
#define RT_SHADE_NOFLAGS                0  /* clear feature flags          */
#define RT_SHADE_LIGHTING               1  /* need lighting                */
#define RT_SHADE_PHONG                  2  /* need phong shading           */
#define RT_SHADE_TEXTURE_MAPS           4  /* need texture mapping         */
#define RT_SHADE_MIPMAP                 8  /* need mip-maps                */
#define RT_SHADE_REFLECTION            16  /* need reflections             */
#define RT_SHADE_REFRACTION            32  /* need refraction              */
#define RT_SHADE_SHADOWS               64  /* need shadows                 */
#define RT_SHADE_VOLUMETRIC           128  /* need volume rendering        */
#define RT_SHADE_ANTIALIASING         256  /* need antialiasing            */
#define RT_SHADE_DEPTH_OF_FIELD       512  /* need depth of field          */
#define RT_SHADE_SOFT_SHADOW         1024  /* need soft-shadows/penumbra   */
#define RT_SHADE_VOLUMETRIC_SHADOW   2048  /* need volumetric shadows      */
#define RT_SHADE_CLIPPING            4096  /* need clipping logic enabled  */
#define RT_SHADE_AMBIENTOCCLUSION    8192  /* need ambient occlusion       */


/*
 * Background texture mapping types
 */
#define RT_BACKGROUND_TEXTURE_SOLID        0
#define RT_BACKGROUND_TEXTURE_SKY_SPHERE   1


/* 
 * Object texture mapping types
 */
#define RT_TEXTURE_CONSTANT                0
#define RT_TEXTURE_3D_CHECKER              1
#define RT_TEXTURE_GRIT                    2
#define RT_TEXTURE_MARBLE                  3
#define RT_TEXTURE_WOOD                    4
#define RT_TEXTURE_GRADIENT                5
#define RT_TEXTURE_CYLINDRICAL_CHECKER     6
#define RT_TEXTURE_CYLINDRICAL_IMAGE       7
#define RT_TEXTURE_SPHERICAL_IMAGE         8
#define RT_TEXTURE_PLANAR_IMAGE            9
#define RT_TEXTURE_VOLUME_IMAGE           10


/*
 * Phong types
 */
#define RT_PHONG_PLASTIC                   0
#define RT_PHONG_METAL                     1


/*
 * Supported output file formats list.
 */
/* 24-bit color image formats */
#define RT_FORMAT_TARGA                    0
#define RT_FORMAT_PPM                      1
#define RT_FORMAT_SGIRGB                   2
#define RT_FORMAT_JPEG                     3
#define RT_FORMAT_WINBMP                   4
#define RT_FORMAT_PNG                      5

/* deeper image formats */
#define RT_FORMAT_PPM48                    6
#define RT_FORMAT_PSD48                    7


/*
 * Image cropping modes
 */
#define RT_CROP_DISABLED                   0
#define RT_CROP_ENABLED                    1


