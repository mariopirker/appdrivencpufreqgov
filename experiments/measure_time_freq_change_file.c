#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

#define __NR_sys_cpufreq_set_frequency 332
#define __NR_sys_cpufreq_set_governer 333

int main(int argc,char *argv[])
{
	struct timeval begin, end;
	const char* userspace_governor = "userspace";
	int userspace_governor_len = strlen(userspace_governor) + 1;
	char *target_frequency = argv[1];
	int cpu_id = atoi(argv[2]);

	FILE *fp;
	int i;
	int size = 100000;
	double system_calls_test1[size];


	printf("[+] Measuring average time of writing to sys-fs...\n");
	syscall(__NR_sys_cpufreq_set_governer, userspace_governor, userspace_governor_len, cpu_id);
	for(i = 0; i < size; i++) 
	{
		/* Start Measurment */
		gettimeofday(&begin, NULL);
		fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "w");
		fprintf(fp,"%s", target_frequency);
		fclose(fp);
		gettimeofday(&end, NULL);
		/* End Measurment */
	
		unsigned int t = end.tv_usec - begin.tv_usec;
		double t2 = (double)t / 1000000;
		
		system_calls_test1[i] = t2;

	}

	double system_calls_test1_average; 
	for (int i = 0; i < size; i++)
	{
		system_calls_test1_average += system_calls_test1[i];
	}

	printf("[+] Average time (samples %d): %f...\n", i, system_calls_test1_average / i); // in seconds!
}
