/*
 * 
 * This file is part of the ALPBench Benchmark Suite Version 1.0
 * 
 * Copyright (c) 2005 The Board of Trustees of the University of Illinois
 * 
 * All rights reserved.
 * 
 * ALPBench is a derivative of several codes, and restricted by licenses
 * for those codes, as indicated in the source files and the ALPBench
 * license at http://www.cs.uiuc.edu/alp/alpbench/alpbench-license.html
 * 
 * The multithreading and SSE2 modifications for SpeechRec, FaceRec,
 * MPEGenc, and MPEGdec were done by Man-Lap (Alex) Li and Ruchira
 * Sasanka as part of the ALP research project at the University of
 * Illinois at Urbana-Champaign (http://www.cs.uiuc.edu/alp/), directed
 * by Prof. Sarita V. Adve, Dr. Yen-Kuang Chen, and Dr. Eric Debes.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal with the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the names of Professor Sarita Adve's research group, the
 *       University of Illinois at Urbana-Champaign, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 * SOFTWARE.
 * 
 */


/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * fillpen.c -- Filler penalties (penalties for words that do not show up in
 * the main LM.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 20-Apr-2001  Ricky Houghton (ricky.houghton@cs.cmu.edu or rhoughton@mediasite.com)
 *              Added fillpen_free to free memory allocated by fillpen_init
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Removed language weight application to wip. To maintain
 *		comparability between s3decode and current decoder. Does
 *		not affect decoding performance.
 *
 * 24-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: Applied language weight to word insertion penalty.
 * 
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include "fillpen.h"
#include "logs3.h"


fillpen_t *fillpen_init (dict_t *dict, char *file, float64 silprob, float64 fillprob,
			 float64 lw, float64 wip)
{
    s3wid_t w, bw;
    float64 prob;
    FILE *fp;
    char line[1024], wd[1024];
    int32 k;
    fillpen_t *_fillpen;
    
    _fillpen = (fillpen_t *) ckd_calloc (1, sizeof(fillpen_t));
    
    _fillpen->dict = dict;
    _fillpen->lw = lw;
    _fillpen->wip = wip;
    if (dict->filler_end >= dict->filler_start)
	_fillpen->prob = (int32 *) ckd_calloc (dict->filler_end - dict->filler_start + 1,
					       sizeof(int32));
    else
	_fillpen->prob = NULL;
    
    /* Initialize all words with filler penalty (HACK!! backward compatibility) */
    prob = fillprob;
    for (w = dict->filler_start; w <= dict->filler_end; w++)
#if 0 /* Language weight need not apply to wip */
	_fillpen->prob[w - dict->filler_start] = (int32) ((logs3(prob) + logs3(wip)) * lw); 
#endif
	_fillpen->prob[w - dict->filler_start] = (int32) ((logs3(prob)*lw + logs3(wip))); 

    /* Overwrite silence penalty (HACK!! backward compatibility) */
    w = dict_wordid (dict, S3_SILENCE_WORD);
    if (NOT_S3WID(w) || (w < dict->filler_start) || (w > dict->filler_end))
	E_FATAL("%s not a filler word in the given dictionary\n", S3_SILENCE_WORD);
    prob = silprob;
#if 0 /* language weight need not apply to wip */
   _fillpen->prob[w - dict->filler_start] = (int32) ((logs3(prob) + logs3(wip)) * lw); 
#endif
   _fillpen->prob[w - dict->filler_start] = (int32)((logs3(prob)*lw + logs3(wip)));
    
    /* Overwrite with filler prob input file, if specified */
    if (! file)
	return _fillpen;
    
    E_INFO("Reading filler penalty file: %s\n", file);
    if ((fp = fopen (file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')	/* Skip comment lines */
	    continue;
	
	k = sscanf (line, "%s %lf", wd, &prob);
	if ((k != 0) && (k != 2))
	    E_FATAL("Bad input line: %s\n", line);
	w = dict_wordid(dict, wd);
	if (NOT_S3WID(w) || (w < dict->filler_start) || (w > dict->filler_end))
	    E_FATAL("%s not a filler word in the given dictionary\n", S3_SILENCE_WORD);
	
#if 0 /* language weight need not apply to wip */
        _fillpen->prob[w - dict->filler_start] = (int32) ((logs3(prob) + logs3(wip)) * lw); 
#endif
        _fillpen->prob[w - dict->filler_start] = (int32) ((logs3(prob)*lw + logs3(wip)));
    }
    fclose (fp);
    
    /* Replicate fillpen values for alternative pronunciations */
    for (w = dict->filler_start; w <= dict->filler_end; w++) {
	bw = dict_basewid (dict, w);
	if (bw != w)
	    _fillpen->prob[w-dict->filler_start] = _fillpen->prob[bw-dict->filler_start];
    }
    
    return _fillpen;
}


int32 fillpen (fillpen_t *f, s3wid_t w)
{
    assert ((w >= f->dict->filler_start) && (w <= f->dict->filler_end));
    return (f->prob[w - f->dict->filler_start]);
}


/* RAH, free memory allocated above */
void fillpen_free (fillpen_t *f)
{
  if (f) {
    if (f->prob) 
      ckd_free ((void *) f->prob);
    ckd_free ((void *) f);
  }
}
