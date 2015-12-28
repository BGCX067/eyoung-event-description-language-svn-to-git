#ifndef _SHARED_H_
#define _SHARED_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <CL/opencl.h>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc __mingw_aligned_malloc 
#define _aligned_free  __mingw_aligned_free 
#endif // __MINGW32__  and __MINGW64_VERSION_MAJOR

#include <malloc.h>


#define ASSERT_CL_RETURN( ret )\
   if( (ret) != CL_SUCCESS )\
   {\
      fprintf( stderr, "%s:%d: error: %s\n", \
             __FILE__, __LINE__, cluErrorString( (ret) ));\
      exit(-1);\
   }

extern cl_mem_flags inFlags;
extern cl_mem_flags outFlags;
extern cl_mem_flags copyFlags;

extern struct _flags { cl_mem_flags f;
                       const char  *s; } flags[];
extern int nFlags;

extern cl_command_queue queue;
extern cl_context       context;
extern cl_kernel        read_kernel;
extern cl_kernel        write_kernel;
extern cl_uint          deviceMaxComputeUnits;
extern int              devnum;
extern char             devname[];

const char *cluErrorString(cl_int);
cl_int      spinForEventsComplete( cl_uint, cl_event * );
void        initCL( char * );

#endif // _SHARED_H_
