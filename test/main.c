#include <cups/cups.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *agrv[]) {
//	int fd = open("/home/cww/tt", O_CREAT | O_RDWR);
//	write(fd, "123", 4);
//	close(fd);
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

	/*
	int job_id;
	//if ((job_id = cupsPrintFile("Brother_HL-1208_Printer:4", "/home/cww/pcms-0.01/test/main.c", "title_test" ,0, NULL)) == 0) {
	if ((job_id = cupsPrintFile("CUPS-PDF", "/home/cww/Desktop/copy2.ps", "copy2_test" ,0, NULL)) == 0) {
		//perror("cupsPrintFile() error");
		puts(cupsLastErrorString());
		exit(1);
	}
	printf("job_id = %d\n", job_id);
	*/
	return 0;
}
