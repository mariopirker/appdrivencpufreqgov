#include<stdio.h>
#include<string.h>
#define __NR_sys_cpufreq_set_frequency 329
#define __NR_sys_cpufreq_set_governer 330

int main(int argc,char *argv[])
{	
	syscall(__NR_sys_cpufreq_set_governer,"userspace",strlen("userspace")+1,atoi(argv[2]));
	syscall(__NR_sys_cpufreq_set_frequency,argv[1],strlen(argv[1])+1,atoi(argv[2]));
}
