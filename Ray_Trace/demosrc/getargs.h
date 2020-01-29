

typedef struct {
  char **filenames;               /* list of model files to render */
  int numfiles;                   /* number of files to render */
  int useoutfilename;             /* command line override of output filename */
  char outfilename[FILENAME_MAX]; /* name of output image file */
  int outimageformat;             /* format of output image */
  int verbosemode;                /* verbose flags */
  int aa_maxsamples;              /* antialiasing setting */
  int boundmode;                  /* bounding mode */
  int boundthresh;                /* bounding threshold */
  int usecamfile;                 /* use camera file */
  char camfilename[FILENAME_MAX]; /* camera filename */
  int shadermode;                 /* quality level */
  int phongfunc;                  /* shader for specular highlights */
  int transmode;                  /* transparency rendering mode */
  int fogmode;                    /* fog rendering mode */
  int numthreads;                 /* explicit number of threads to use */
  int nosave;                     /* don't write output image to disk */
  int xsize;                      /* override default image x resolution */
  int ysize;                      /* override default image y resolution */
  int normalfixupmode;            /* override normal fixup mode */
  int imgprocess;                 /* image post processing flags */
  float imggamma;                 /* image gamma correction factor */
  float rescale_lights;           /* rescale all lights by supplied factor */
  float auto_skylight;            /* automatic ambient occlusion lighting */
  float add_skylight;             /* manual ambient occlusion lighting */
  int skylight_samples;           /* number of ambient occlusion samples */
  char spaceball[FILENAME_MAX];   /* spaceball serial port device */
  int cropmode;                   /* post rendering image crop (SPECMPI) */
  int cropxres;
  int cropyres;
  int cropxstart;
  int cropystart;
} argoptions;

int getargs(int argc, char **argv, argoptions * opt, int node);
int presceneoptions(argoptions * opt, SceneHandle scene);
int postsceneoptions(argoptions * opt, SceneHandle scene);
void freeoptions(argoptions * opt);
