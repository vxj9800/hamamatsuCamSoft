// console/misc/common.h
//

#include	"console4.h"
#include	<stdarg.h>

#ifndef ASSERT
#define	ASSERT(c)
#endif

void dcamcon_show_dcamerr( HDCAM hdcam, DCAMERR errid, const char* apiname, const char* fmt=0, ...  );

HDCAM dcamcon_init_open();
void dcamcon_show_dcamdev_info( HDCAM hdcam );

//---

BOOL console_prompt(const char* prompt, char* buf, int32 bufsize );
void output_data( const char* filename, char* buf, int32 bufsize );
