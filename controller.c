#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define __NR_sys_cpufreq_set_frequency 332
#define __NR_sys_cpufreq_set_governer 333

int main(void)
{
	int 		fd_in, fd_out;
	int		nbytes;
	pid_t		childpid;
	char		readbuffer[80];
	char string[] 	= "Test\n";
	char pipe_in[] 	= "/tmp/hb_in";
	char pipe_out[] = "/tmp/hb_out";
	char *end;

	/* Videos played with a heartbeat rate of 24 yield to acceptable results */
	float heartbeat_low = 20.00;
	float heartbeat_upper = 24.00;
	float heartbeat_current = 0.00;
	float heartbeat_global = 0.00;

	/* Controller Settings */
	int timer = 0.1; // the controller loop will sleep for "timer" seconds for each iteration

	/* Read cpuinfo_min_freq and cpuinfo_max_freq from sysfs */
	int cpu_id = 0;
	int freq_step = 100000; // the controller will increase/decrease the frequency in steps of this
	int cur_cpu_freq, min_cpu_freq, max_cpu_freq;
	char *str_cur_cpu_freq = malloc(32 * sizeof(char));
	char *str_min_cpu_freq = malloc(32 * sizeof(char));
	char *str_max_cpu_freq = malloc(32 * sizeof(char));
	FILE *pfile_min = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "r");
	FILE *pfile_max = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");

	if(pfile_min == NULL)
	{
		perror("Error opening file /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
		return(-1);
	}

	if(pfile_max == NULL)
	{
		perror("Error opening file /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
		return(-1);
	}

	fscanf(pfile_min, "%d", &min_cpu_freq);
	fscanf(pfile_max, "%d", &max_cpu_freq);

	printf("[+] Controller starting...\n");
	printf("[+] Reading cpuinfo_min_freq from procfs for cpu0: %d Hz...\n", min_cpu_freq);
	printf("[+] Reading cpuinfo_max_freq from procfs for cpu0: %d Hz...\n", max_cpu_freq);
	printf("[+] Heartbeat lower boundary set to: %.2f...\n", heartbeat_low);
	printf("[+] Heartbeat upper boundary set to: %.2f...\n", heartbeat_upper);
	printf("[+] CPU frequency gets increased in steps of %d Hz...\n", freq_step);

	sprintf(str_min_cpu_freq, "%d", min_cpu_freq);
	sprintf(str_max_cpu_freq, "%d", max_cpu_freq);
	printf("[+] Enabling userspace governor...\n");
	printf("[+] Setting CPU frequency of cpu0 to %d Hz...\n", min_cpu_freq);
	syscall(__NR_sys_cpufreq_set_governer, "userspace", strlen("userspace") + 1, cpu_id);
	syscall(__NR_sys_cpufreq_set_frequency, str_min_cpu_freq, strlen(str_min_cpu_freq) + 1, cpu_id);

	printf("[+] Entering monitoring loop...\n");
	printf("[+] Press CTRL+C to exit...\n");		
	cur_cpu_freq = min_cpu_freq;
	while(1)
	{
		end = NULL;
		/* Writing to pipe_in triggers the heartbeat listener in ffplay */
		fd_in = open(pipe_in, O_WRONLY);
		write(fd_in, string, strlen(string) + 1);

		fd_out = open(pipe_out, O_RDONLY);
		nbytes = read(fd_out, readbuffer, 80);
		readbuffer[nbytes] = '\0';
	
		heartbeat_current = (float) strtol(readbuffer, &end, 10);	
		printf("\t [+] Heartbeat: %.2f\n", heartbeat_current);

		if(heartbeat_current >= heartbeat_upper)
		{
			/* Decrease frequency */
			if(cur_cpu_freq == min_cpu_freq)
			{
				printf("\t\t [!] Cannot decrease frequency, already set to min_cpu_freq = %d\n", min_cpu_freq);
			}	
			else if((cur_cpu_freq - freq_step) < min_cpu_freq) 
			{
				printf("\t\t [!] Decreasing frequency to min_cpu_freq = %d\n", min_cpu_freq);
				cur_cpu_freq = min_cpu_freq;
				syscall(__NR_sys_cpufreq_set_frequency, str_min_cpu_freq, strlen(str_min_cpu_freq) + 1, cpu_id);
			}
			else
			{
				cur_cpu_freq = cur_cpu_freq - freq_step;
				sprintf(str_cur_cpu_freq, "%d", cur_cpu_freq);
				printf("\t\t [!] Decreasing frequency to %s\n", str_cur_cpu_freq);
				syscall(__NR_sys_cpufreq_set_frequency, str_cur_cpu_freq, strlen(str_cur_cpu_freq) + 1, cpu_id);
			}
		}
		else if(heartbeat_current <= heartbeat_low)
		{
			/* Increase frequency */
			if(cur_cpu_freq == max_cpu_freq)
			{
				printf("\t\t [!] Cannot increase frequency, already set to max_cpu_freq = %d\n", max_cpu_freq);
			}
			else if((cur_cpu_freq + freq_step) > max_cpu_freq)
			{
				printf("\t\t [!] Increasing frequency to max_cpu_freq = %d\n", max_cpu_freq);
				cur_cpu_freq = max_cpu_freq;
				syscall(__NR_sys_cpufreq_set_frequency, str_max_cpu_freq, strlen(str_max_cpu_freq) + 1, cpu_id);
			}
			else
			{	
				cur_cpu_freq = cur_cpu_freq + freq_step;
				sprintf(str_cur_cpu_freq, "%d", cur_cpu_freq);
				printf("\t\t [!] Increasing frequency to %s\n", str_cur_cpu_freq);
				syscall(__NR_sys_cpufreq_set_frequency, str_cur_cpu_freq, strlen(str_cur_cpu_freq) + 1, cpu_id);
				
			}
		}
		
		sleep(timer);
		
	}

	return(0);
}
