#ifndef _UNICAS_CONFIG_H
#define _UNICAS_CONFIG_H

#define NOMINMAX

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "ucasFileUtils.h"
#include "ucasStringUtils.h"
#include "ucasMultithreading.h"
#include "ucasMathUtils.h"
#include "ucasImageUtils.h"
#include "ucasBreastUtils.h"
#include "ucasExceptions.h"
#include "ucasLog.h"
#include "ucasStringUtils.h"
#include "ucasTypes.h"


/**************************************************************************************************************************
 *   Cross-platform UTILITY functions	   																			      *
 **************************************************************************************************************************/
//file deleting
#ifdef _WIN32
#define RM_FILE( arg ) 						\
	sprintf(ucas::sys_cmd, "del /F /Q /S \"%s\"", arg);  \
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't delete file \"%s\"\n", arg);
#else
#define RM_FILE( arg )						\
	sprintf(ucas::sys_cmd, "rm -f \"%s\"", arg);		\
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't delete file \"%s\"\n", arg);
#endif

//recycling
#ifdef _WIN32
#define RECYCLE( arg ) 						\
	sprintf(ucas::sys_cmd, "recycle \"%s\"", (arg));  \
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't recycle file \"%s\"\n", (arg));
#else
#define RECYCLE( arg ) \
	fprintf(stderr,"RECYCLE macro not defined on this system\n");
#endif

//"PAUSE" function
#ifdef _WIN32
#define system_PAUSE() system("PAUSE"); cout<<endl;
#define system_CLEAR() system("cls");
#else
#define system_CLEAR() system("clear");
#define system_PAUSE()										\
	cout<<"\n\nPress RETURN key to continue..."<<endl<<endl;	\
	system("read");
#endif

//FILE COPY
#ifdef _WIN32
#define COPY_ASCII_FILE( src, dst ) 						\
	sprintf(ucas::sys_cmd, "copy /Y /A \"%s\" \"%s\"", src, dst);  \
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't copy file from \"%s\" to \"%s\"\n", src, dst);
#else
#define COPY_ASCII_FILE( src, dst ) \
	fprintf(stderr,"COPY_ASCII_FILE macro not defined on this system\n");
#endif
#ifdef _WIN32
#define COPY_BINARY_FILE( src, dst ) 						\
	sprintf(ucas::sys_cmd, "copy /Y /B \"%s\" \"%s\"", src, dst);  \
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't copy file from \"%s\" to \"%s\"\n", src, dst);
#else
#define COPY_BINARY_FILE( src, dst ) \
	fprintf(stderr,"COPY_BINARY_FILE macro not defined on this system\n");
#endif

//FOLDER COPY
#ifdef _WIN32
#define COPY_FOLDER( src, dst ) 						\
	sprintf(ucas::sys_cmd, "xcopy /Y /S /Q /I \"%s\" \"%s\"", src, dst);  \
	if(system(ucas::sys_cmd)!=0)					\
	fprintf(stderr,"Can't copy directory from \"%s\" to \"%s\"\n", src, dst);
#else
#define COPY_FOLDER( src, dst ) \
	fprintf(stderr,"COPY_FOLDER macro not defined on this system\n");
#endif

#endif //_MCD_CONFIG_H
