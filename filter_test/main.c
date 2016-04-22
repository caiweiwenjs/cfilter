/*
#include <cups/cups.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define PS_GSAVE "gsave "
#define PS_GRESTORE "grestore "
#define PS_FIND_FONT "/Times-Roman findfont "
#define PS_SET_FONT_SIZE "12 scalefont "
#define PS_SET_FONT "setfont "
#define PS_SET_GRAY ".5 setgray "
#define PS_TRANSLATE "0 0 translate "
#define PS_SET_POS "100 500 moveto "
#define PS_SHOW "show "
//space is requisite
#define PS_SHOWPAGE "showpage"

void error_msg(char *str) {
	fprintf(stderr, "ERROR: %s\n", str);
}

void fputs_flush(char *str, FILE *stream) {
	fputs(str, stream);
	fflush(stream);
	error_msg(str);
}

void inject(char *str) {
	fputs_flush(PS_GSAVE, stdout);
	fputs_flush(PS_FIND_FONT, stdout);
	fputs_flush(PS_SET_FONT_SIZE, stdout);
	fputs_flush(PS_SET_FONT, stdout);
	fputs_flush(PS_SET_GRAY, stdout);
	fputs_flush(PS_TRANSLATE, stdout);
	fputs_flush(PS_SET_POS, stdout);
	fputs_flush(str, stdout);
	fputs_flush(PS_SHOW, stdout);
	fputs_flush(PS_GRESTORE, stdout);
}

int main(int argc, char *agrv[]) {
//	FILE *fp = fopen("/tmp/tt", "w");
	//write(fd, "123", 4);
//	if (fp == NULL)
//		fprintf(stderr, "ERROR: %s\n", strerror(errno));
	char str[4096];
	while (fgets(str, sizeof(str), stdin) != NULL) {
		char *find_pos;
		if ((find_pos = strstr(str, PS_SHOWPAGE)) != NULL && 
					(find_pos == str || *(find_pos - 1) != '/')) {
			if (find_pos != str) {
				char chr = *find_pos;
				*find_pos = '\0';
				fputs_flush(str, stdout);
				*find_pos = chr;
			}
			inject("(My name is cww, I'm 26 year old.)");
			fputs_flush(find_pos, stdout);
		} else if ((find_pos = strstr(str, PS_SHOWPAGE)) != NULL &&
					(find_pos != str && *(find_pos - 1) == '/')) {
			inject("(My name is cww, I'm 26 year old.)");
			fputs_flush(str, stdout);
		} else { 
			fputs_flush(str, stdout);
		}
	}
//	fclose(fp);
	/*
	cups_dest_t *dests;

	int num_dests = cupsGetDests(&dests);

	int i;
	for (i = 0;i < num_dests;i++) {
		cups_dest_t *t = dests + i;
		if (t != NULL) {
			printf("#%d:name = %s, instance=%s\n", i, t->name, t->instance);
		}
	}

	printf("num_dests = %d\n", num_dests);
	cups_dest_t *dest = cupsGetDest(NULL, NULL, num_dests, dests);

	if (dest != NULL) {
		printf("name = %s, instance = %s\n", dest->name, dest->instance);
	}

	cupsFreeDests(num_dests, dests);
*/
//	return 0;
//}
