#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50

char history[10][BUFFER_SIZE];
int count = 0;  /* Number of commands entered*/
char hist_array[50]; /*Holds the commands fetched from history on request*/


void printHistory()
{
	int i,k;
	int j = 0;
	int count_duplicate = count;
	for (i = 0; i<10;i++)
	{
		 k=0;
  		 printf("%d.  ", count_duplicate);
		 while (history[i][j] != '\n' && history[i][j] != '\0')
		 {
			  printf("%c", history[i][j]);
			  j++;
		 }
		 printf("\n");
		 j = 0;
		 count_duplicate--;
		 if (count_duplicate ==  0)
			break;
	}
	printf("\n");


} 

/*
  setupCommand() reads in the next command line, separating it into distinct tokens
using whitespace as delimiters. setupCommand() sets the args parameter as a
 null-terminated string.
 */


void setupCommand(char in_buffer[], char *args[],int *background)
{
	int length; /* # of charaindexers in the command line */
	int i;     /* loop index for accessing in_buffer array */
	int index_beginning;  /* index where beginning of next command parameter is */
	int index;     /* index of where to place the next parameter into args[] */
	index = 0;
	char histFlag[7];
	/* read what the user enters on the command line */
	length = read(STDIN_FILENO, in_buffer, MAX_LINE);

	/*logic to not insert !! and !N in history array*/
	if(length >=7)
	{
		memset(&histFlag, 0, sizeof(histFlag)); /* Array is reset*/
		strncpy ( histFlag, in_buffer, 7 );
	}

	if(in_buffer[0] != '!' && strcmp("history",histFlag)!=0) 
	{
		for (i = 9;i>0; i--)
			strcpy(history[i], history[i-1]);
		strcpy(history[0],in_buffer);
		count++;
	}
	index_beginning = -1;
	if (length == 0)
		exit(0);            

	if (length < 0)
	{
		perror("error reading the command\n");
		exit(-1);           /* if length is less than zero terminate with an error*/
	}

	/* Checking every character in the in_buffer */
	for (i=0;i<length;i++)
	{
		switch (in_buffer[i])
		{
			case ' ':
			case '\t' :               /* tabs is argument separators */
				if(index_beginning != -1)
				{
					args[index] = &in_buffer[index_beginning];    /* set up pointer */
					index++;
				}
				in_buffer[i] = '\0'; /* adding a null char to make a C string */
				index_beginning = -1;
			break;

			case '\n':                 /* endline character is final character */
				if (index_beginning != -1 && *background==0)
				{
					args[index] = &in_buffer[index_beginning];
					index++;
				}
				in_buffer[i] = '\0';
				args[index] = NULL; /* Indicates end of command */
			break;

			default :             /* any other character */
				if (index_beginning == -1)
					index_beginning = i;
				if (in_buffer[i] == '&')
				{
					*background  = 1;
					in_buffer[i] = '\0';
				}
		}
	}
	args[index] = NULL; /* if it exceeds size of 80 */
}

int main(void)
{
	system("clear");
	char in_buffer[MAX_LINE]; /* to store the input commands*/
	int background;             /*is set if '&' is given at end of command */
	char *args[MAX_LINE/+1];/* to have 40 arguments */
	char checkCommand[10]; /* To track the input like !! and !N*/
	pid_t pid;
	int i;
	int shouldrun = 1;
	while (shouldrun)  //while runs always, unless user enter exit
	{           
		background = 0;
		printf(" COMMAND->\n");
		setupCommand(in_buffer,args,&background);  /* To get next command */
		if(strcmp(args[0],"exit")==0)	//if user enters exit, it exits out
		{
			printf("exiting\n");
			shouldrun=0;
			exit(0);
		}
		int k,j,status_flag;
		k = 0;
		j=0;
		int histCounter =0; /* This is loop counter to traverse through history array*/
		int lengthArgs = strlen(args[0]); 
		status_flag=0; /* This is to track whether command entered will be executed or not by execvp*/
		int no_commands=0;
		int hist_flag = 0; /* This flag is to test whether !! or !N has been given as input or not*/
		int maxIndex = count;
		int minIndex =0;  
		if(count>9)
		minIndex = count - 9;
		char* histHold[20];
		strcpy(checkCommand,args[0]);
		int position_value = 0; /* This will hold what position will fetched from history */
		if(lengthArgs<=2)
			position_value = (int)checkCommand[1] - '0'; /* Convert the character digit to numeric value*/

		/**
			*This else loop will work in this way: if !10 is given then position to be fetched from is 10
			* It will extract the value given after '!' and will store in variable position_value
		*/		
		else
		{
			int power = lengthArgs -2;	
			int value = 0;
			int loop=0;
			int hold;
			/**
				* This for loop takes value N from !N
				* It then parse each digit from N and forms the numeric value
				* If input is given as !10 then it will take out 12
				* It will parse each digit (1 and 0)
				* It will form the numeric value as 1*(10 power of 1) + 2 * (10 power of 0)
			*/	
			for(loop=1;loop<lengthArgs;loop++)
			{
				hold = (int)checkCommand[loop] -'0';
				value = value + hold * pow(10,power);
				power--;
			}
			position_value = value;
		}
		/* This block will fetch the command from history only when the input pattern will be like !! or !N*/
		if(checkCommand[0] == '!' )
		{
			if((checkCommand[1] == '!' && count>0) || 
				(position_value >= minIndex 
					&& position_value <= maxIndex))
			{
				int position = 0; /* This is initialized to 0 to get the latest command from history*/
				/*Check for the pattern !N*/
				if(checkCommand[1]!='!')
				{
					position =  maxIndex - position_value;
				}
				memset(&hist_array[0], 0, sizeof(hist_array)); /* Array initialization*/
				/**
					* This while loop will take each character (except null and enter) from history array
					* It will put the character in a temporary 1D array
				*/
				while (history[position][j] != '\n' && history[position][j] != '\0')
				{
					hist_array[k] = history[position][j];
					j++;
					k++;
				}

				/**
					*This is to make an entry in the history array 
					* with the command corresponding to the position !! 
					* or !N
				*/
				for (histCounter = 9;histCounter>0; histCounter--)
					strcpy(history[histCounter], history[histCounter-1]);

				strcpy(history[0],hist_array);
				count++;
				printf("Command fetched : %s\n",hist_array);

				/** 
					* This block will get the command from the history for the given position.
					* Commands are spliited by space if it contains more than one word
					* It will store the command in a pointer of character array
					* The pointer character array is passed to execvp to execute
				*/
				int hist_array_length = strlen(hist_array); /* Get the length of the command fetched from history*/
				int arrayCounter = 0;
				char holdCharacter; /* This will hold the each character of the command while parsing the command*/
				int histHoldCounter = 0;
				int blankPossible = 0; /* This is handle multiple blanks between words in a command*/
				histHold[histHoldCounter] = (char*) calloc(1, sizeof(char)); /* Dynamic Initialization*/
				/**
					* This for loop will parse the temporary 1D array
					* characters are concatenated and put in the current index of array of character pointers
					* If the character is space then next index of pointer array is initialized
				*/
				for(arrayCounter =0 ;arrayCounter<hist_array_length;arrayCounter++)
				{
					holdCharacter = hist_array[arrayCounter];
					if(holdCharacter != ' ')
					{
						histHold[histHoldCounter] = strcat(histHold[histHoldCounter],&holdCharacter);
						blankPossible = 1;						
					}
					if(holdCharacter == ' ')
					{
						/*handle multiple blank*/
						if(blankPossible == 1 )
						{
							histHoldCounter++;
							histHold[histHoldCounter] = (char*) calloc(1, sizeof(char));
							blankPossible = 0;
						}
					}
				}
				/* Set the immediate next index as zero after populating it with the parsed command words*/
				histHold[histHoldCounter + 1 ] = NULL; 
				status_flag = 1;
				hist_flag = 1;
				/* Block ends here */ 
			}
			else 
			{
				if(count == 0)
					printf("No commands in history\n");
				else 
				{
					printf("No such command in histoy\n");
					no_commands = 1; //flag used to stop creating processes if there are no such commands to execute
				}
				status_flag = 1; //flag used to stop executing execvp if there are no commands to execute
			}
		}
		/* This block is to print the history of commands when user give input command as 'history'*/
		if(strcmp("history",args[0])==0) 
		{
			if(count>0)
			{
				printHistory();
				status_flag = 1;
			}else
			{
				printf("No commands in history\n");
				status_flag = 1;
			}

		}
		if((strcmp("history",args[0])!=0) && (no_commands == 0)) //conditions to check when to create processes
			pid = fork();
		int pid_child = 0;
		if (pid < 0)
		{
			printf("Fork failed.\n");
			exit (1);
		}

		/*the child process will invoke execvp()*/
		else if (pid == 0)
		{
			pid_child = getpid(); //getting the pid of child process
			if(hist_flag == 1)
			{
				status_flag = 1;
				execvp (histHold[0], histHold); //executing of command fetched from history
			}
			if(status_flag == 0) 
			{
				if (execvp (args[0], args) == -1) //checking whether command has executed properly or not. If it cannot execute, displaying error executing command
				{
					printf("Error executing command\n"); 
					kill(pid_child,SIGKILL);//if command gives an error, the child process has to be killed explicitly. If not, program won't exit when exit command is given.
				}
			}
		}

		/**
			*if background == 0, the parent will wait,
			* else returns to the setupCommand() function.
		*/
		else
		{
			if (background == 0)
			waitpid(pid,NULL,0);
		}
	}
}
