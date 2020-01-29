/* 
 * triangle.h - This file contains the defines for triangles etc.
 *
 *  $Id: triangle.h,v 1.20 2009/05/27 18:32:30 johns Exp $
 */

object * newtri(void *, vector, vector, vector);
object * newstri(void *, vector, vector, vector, vector, vector, vector);
void stri_normal_fixup(object *, int mode);
object * newvcstri(void *, vector, vector, vector, vector, vector, vector,
                   color, color, color);
void vcstri_normal_fixup(object *, int mode);
color vcstri_color(const vector * hit, const texture * tex, const ray * incident);

#ifdef TRIANGLE_PRIVATE

#define TRIXMAJOR 0
#define TRIYMAJOR 1
#define TRIZMAJOR 2
 
typedef struct {
  RT_OBJECT_HEAD
  vector edge2;
  vector edge1;
  vector v0;
} tri; 

typedef struct {
  RT_OBJECT_HEAD
  vector edge2;
  vector edge1;
  vector v0;
  vector n0;
  vector n1;
  vector n2;
} stri; 

typedef struct {
  RT_OBJECT_HEAD
  vector edge2;
  vector edge1;
  vector v0;
  vector n0;
  vector n1;
  vector n2;
  color  c0;
  color  c1;
  color  c2;
} vcstri; 

static int tri_bbox(void * obj, vector * min, vector * max);

static void tri_intersect(const tri *, ray *);

static void tri_normal(const tri *, const vector *, const ray *, vector *);
static void stri_normal(const stri *, const vector *, const ray *, vector *);
static void stri_normal_reverse(const stri *, const vector *, const ray *, vector *);
static void stri_normal_guess(const stri *, const vector *, const ray *, vector *);

#endif


