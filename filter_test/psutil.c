#include <stdio.h>
#include "psutil.h"


void fputs_flush(char *str, FILE *stream) {
	fputs(str, stream);
	fflush(stream);
}

void inject(char *str) {
	fputs_flush(PS_GSAVE, stdout);
	fputs_flush(PS_FIND_FONT, stdout);
	fputs_flush(PS_SET_FONT_SIZE, stdout);
	fputs_flush(PS_SET_FONT, stdout);
	fputs_flush(PS_SET_GRAY, stdout);
	fputs_flush(PS_TRANSLATE, stdout);
	fputs_flush(PS_SET_POS, stdout);
	char tstr[1024];
	sprintf(tstr, "(%s)", str);
	fputs_flush(tstr, stdout);
	fputs_flush(PS_SHOW, stdout);
	fputs_flush(PS_GRESTORE, stdout);
}


