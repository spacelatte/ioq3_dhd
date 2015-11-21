///////////////////////////////////////////////////////////////////////////////
//
//  (C) 2001-2010 Force Dimension
//  All Rights Reserved.
//
//  DHD API 3.2
//
///////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
#ifndef CTextureH
#define CTextureH
//---------------------------------------------------------------------------
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef APPLE
#include "GLUT/glut.h"
#else
#include "GL/glut.h"
#endif
//---------------------------------------------------------------------------
unsigned *read_texture(const char *name, int *width, int *height, int *components);
int create_texture(const char *fname, GLsizei *w,  GLsizei *h, GLsizei *padW, GLsizei *padH, int *comps);
void imgLoad(const char *filenameIn, int borderIn, GLfloat borderColorIn[4], int *wOut, int *hOut, GLubyte ** imgOut);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
