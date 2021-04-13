#include <stdio.h>
#include <signal.h>	// signal(), kill(), sigaction()
#include <unistd.h>     // fork()
#include <sys/wait.h>   // wait() 
#include <time.h> 	// timestamp
#include <stdlib.h>	// exit()

//interrupt is generated with ^C by default in my linux version

const int NUM_CHILD = 4;
typedef enum { F, T } boolean;
boolean interruptRecieved = F; 						//Global variable 3.c

#define WITH_SIGNALS							//Comment or uncomment - 2 versions

void sigtermChild(){
	printf("Child[%d] : Parent interrupt signal provided. \n", getpid());
	exit (2);	
}

void sigintParent(){
	interruptRecieved = T;		
	printf("Parent[%d]: Interrupt signal provided. \n", getpid());	
}


int main() {
	int i, j, stat, exits_observed = 0;
	int childPID[NUM_CHILD];
	time_t ltime; 							// calendar time 
	pid_t pid = getpid(), parent = getpid();
	printf("Parent[%d]: Starting operation!\n", parent);

	//Task 3.1
	#ifdef WITH_SIGNALS
	for(i = 0; i < NSIG; ++i) 					//NSIG is number of all signals defined
        {
        	signal(i, SIG_IGN); 					//ignore all signals	
        }
	signal(SIGCHLD, SIG_DFL);					//restore SIGCHLD to default
	signal(SIGINT, sigintParent); 					//my own sigint
	#endif

	for (i=0; i<NUM_CHILD; ++i){ 					//NUM_CHILD of child processes
		if(pid > 0){						//We are in a parent	
			sleep(1);
			
			#ifdef WITH_SIGNALS
			if(interruptRecieved == T){
				for(j=0; j<i; j++){			//Kill only already created children
					if(childPID[j]>=0){
						kill(childPID[j], SIGTERM);
						childPID[j] = -1;	
					}			
				}
				interruptRecieved = F;			//Continue with the loop
			}
			#endif

			pid = fork();					//Task 2.1 - creating child processes
			if (pid < 0)					//Task 2.2 - when child process was not created correctly pid<0
		    	{	
				for(j=0; j<i; j++){			//Kill only already created children
					kill(childPID[i], SIGTERM);
					printf("Parent[%d]: Error in fork() - terminating process! \n", parent);
					exit (1);
				}			
		   	}
			else if(pid > 0){				//is a parent process
				childPID[i] = pid; 			//pid stores child process id
			}else{
				ltime=time(NULL);			//get current time to test delays from 2.1
				printf("Child[%d] : Created at %s", getpid(), asctime(localtime(&ltime)));
				#ifdef WITH_SIGNALS			
					signal(SIGINT, SIG_IGN); 	//ignore SIGINT signals
					signal(SIGTERM, sigtermChild);	//other child termination method from task 3
				#endif
			}
		}
	}


	if(pid!=0){							//parent waits for exit codes
		printf("Parent[%d] : All children created!\n", parent);		
		for (i=0; i<NUM_CHILD; ++i)
		{
			waitpid(childPID[i], &stat, 0);
			if (WIFEXITED(stat) && childPID[i]>=0){
		   		++exits_observed;
			}
	    	}
		printf("Parent[%d]: %d termination codes observed. Shutting down soon!\n", parent, exits_observed);
	}
			
	if(pid == 0){ 
		printf("Child[%d] : Parent ID is %d\n", getpid(), parent);
		sleep(10);						//children go to sleep
		ltime=time(NULL);   					//get current time to test delays	
		printf("Child[%d] : Terminated at %s", getpid(), asctime(localtime(&ltime)));
	}else{
		ltime=time(NULL);					//get current time to test delays
		printf("Parent[%d]: Terminated at %s", getpid(), asctime(localtime(&ltime)));	
	} 

	#ifdef WITH_SIGNALS
	for(i = 0; i < NSIG; ++i) 		//NSIG is number of all signals defined
        {
        	signal(i, SIG_DFL); 		//restore all to default (3e)
        }
	#endif


	return 0;
}
