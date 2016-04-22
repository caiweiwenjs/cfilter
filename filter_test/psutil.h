#ifndef __PSUTIL_H__
#define __PSUTIL_H__

#define PS_GSAVE "gsave "
#define PS_GRESTORE "grestore "
#define PS_FIND_FONT "/Times-Roman findfont "
#define PS_SET_FONT_SIZE "12 scalefont "
#define PS_SET_FONT "setfont "
#define PS_SET_GRAY ".5 setgray "
#define PS_TRANSLATE "0 0 translate "
#define PS_SET_POS "0 2 moveto "
#define PS_SHOW "show "
//space is requisite
#define PS_SHOWPAGE "showpage"

void fputs_flush(char *str, FILE *stream);
void inject(char *str);

#endif
