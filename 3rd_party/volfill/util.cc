#include <iostream>
#include <stdio.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace std;

void PrintResourceUsage(const char *msg) {

	cerr << "volfill info: " << msg << ":";
	
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	char buf[100];
	sprintf(buf, " user %d.%.1ds system %d.%.1ds", ru.ru_utime.tv_sec,
					ru.ru_utime.tv_usec / 100000, ru.ru_stime.tv_sec, 
					ru.ru_stime.tv_usec / 100000);
	
#ifdef linux   
	cerr << buf;
	sprintf(buf, "ps h -o vsize -p %d", getpid());
	FILE *p = popen(buf, "r");
	int vsize;
	fscanf(p, "%d", &vsize);
	fclose(p);
	sprintf(buf, " vsize %.2fMB", vsize / 1024.0);
#endif // linux   

	cerr << buf << endl;
}
