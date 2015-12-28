#ifdef HTML_DEBUG

#include <stdarg.h>
#include "html.h"

int debug_strmengine_decoder_html_basic=1;

int strm_debug(int flag, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

#endif
