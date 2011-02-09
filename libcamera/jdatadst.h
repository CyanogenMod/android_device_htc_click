/*
 * jdatadst.h
 *
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * Modified 2002-2010 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines backported from jpeglib 8 for writing
 * jpeg data directly to memory.
 */

#ifndef JDATADST_H
#define JDATADST_H

/*
 * First we include the configuration files that record how this
 * installation of the JPEG library is set up.  jconfig.h can be
 * generated automatically for many systems.  jmorecfg.h contains
 * manual configuration options that most people need not worry about.
 */

#ifndef JCONFIG_INCLUDED        /* in case jinclude.h already did */
#include <jconfig.h>            /* widely used configuration options */
#endif
//#include <jmorecfg.h>           /* seldom changed options */
#include <string.h>


extern "C" {

/* Declarations for routines called by application.
 * The JPP macro hides prototype parameters from compilers that can't cope.
 * Note JPP requires double parentheses.
 */

#ifdef HAVE_PROTOTYPES
#define JPP(arglist)    arglist
#else
#define JPP(arglist)    ()
#endif


/* Data source and destination managers: memory buffers. */
EXTERN(void) jpeg_mem_dest (j_compress_ptr cinfo,
                               unsigned char ** outbuffer,
                               unsigned long * outsize);
} // extern "C"

#define SIZEOF(object)          ((size_t) sizeof(object))
#define MEMCOPY(dest,src,size)  memcpy((void *)(dest), (const void *)(src), (size_t)(size))

#endif /* JPEGLIB_H */
