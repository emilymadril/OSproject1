	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/wait.h>
	#include <fcntl.h>

	//***********************************************
	typedef struct {
		int size;
		char **items;
	} tokenlist;

	char *get_input(void);
	tokenlist *get_tokens(char *input);

	tokenlist *new_tokenlist(void);
	void add_token(tokenlist *tokens, char *item);
	void free_tokens(tokenlist *tokens);
	//***********************************************


	void pathSearch(tokenlist *tokens, int loc, int *err);
	void replaceTokens (tokenlist *tokens, int *error, tokenlist *paths);
	tokenlist *getPaths(void);

	int path_resolution(char* file);
	void output_redirect(char* file);
	void input_redirect(char* file);

	int pipe_func(tokenlist * token_ptr);

	int check_built(char *);
	void built_in(tokenlist * token_ptr, int x);

	void exit_func(int);
	void change_directory(tokenlist * tknptr);
	void echo(tokenlist * token_ptr);

	int main()
	{
		while (1) {
			int *error = malloc(sizeof(int));
			*error = 0;

			printf("%s@%s : %s> ",getenv("USER"),getenv("MACHINE"),getenv("PWD"));	//Part 3: Prompt

			/* input contains the whole command
			 * tokens contains substrings from input split by spaces
			 */

			char *input = get_input();
			printf("whole input: %s\n", input);


			tokenlist *paths = getPaths();
			tokenlist *tokens = get_tokens(input);
			replaceTokens(tokens, error, paths);

			if (*error == 0)
			{
				for (int i = 0; i < tokens->size; i++)
				{
					printf("token %d: (%s)\n", i, tokens->items[i]);

					if(memchr(tokens->items[i], '/', strlen(tokens->items[i])) == NULL && i == 0)
					{
						pathSearch(tokens, i, error);
					}
				}
			}

			if(check_built(tokens->items[0]) == 0)
			{
				fflush(0);
				execute(&tokens->items[0]);
			}
			else
			{
				built_in(&tokens, tokens->size);
			}

			free(input);
			free_tokens(tokens);
			free_tokens(paths);
		}

		return 0;
	}

	//Part 5-6: Path Search and ls execv
	void pathSearch(tokenlist *tokens, int loc, int *err)
	{
		char *outputFile = NULL;
		char *inputFile = NULL;
		int outputFLG = 0, inputFLG = 0;
		int fd, fdin;
		tokenlist *args = new_tokenlist();
		add_token(args, "temp");
		for (int i = loc + 1; i < tokens->size; i++)
		{
			if (strcmp(tokens->items[i], "<") == 0 && inputFLG == 0)
			{
				if (tokens->items[i+1] == NULL)
				{
					printf("No file to input\n");
					*err = 1;
					break;
				}
				else
				{
					inputFile = (char *) malloc(strlen(tokens->items[i+1] + 1));
					strcpy(inputFile, tokens->items[i+1]);
					inputFLG = 1;
				}
			}
			else if (strcmp(tokens->items[i], ">") == 0 && outputFLG == 0)
			{
				if (tokens->items[i + 1] == NULL)
				{
					printf("No file to output to\n");
					*err = 1;
					break;
				}
				else
				{
					outputFile = (char *) malloc(strlen(tokens->items[i + 1] + 1));
					strcpy(outputFile, tokens->items[i+1]);
					outputFLG = 1;
				}
			}
			if(i != tokens->size && outputFLG == 0 && inputFLG == 0)
				add_token(args, tokens->items[i]);
		}
		printf("outputFile: %s\n", outputFile);
		printf("inputFile: %s\n", inputFile);
		//Storing all the path variables in path
		int found = 0;
		tokenlist *new_list = new_tokenlist();
		char *path = (char *) malloc(strlen(getenv("PATH") + 1));
		strcpy(path, getenv("PATH"));
		//creating list of all the different paths
		char *path_token = strtok(path, ":");
		while (path_token != NULL)
		{
			add_token(new_list, path_token);
			path_token = strtok(NULL, ":");
		}
		//For each path adding on the token to the path
		fd = open(outputFile, O_RDWR | O_CREAT);

		if(path_resolution(inputFile) == 0 && inputFLG == 1)
		{
				*err = 1;
				printf("Error. File does not exist\n");
		}
		else
				fdin = open(inputFile, O_RDWR);

		char* execFile;
		for(int i = 0; i < new_list->size; i++)
		{
			if (*err == 1)
				break;
			execFile = (char*)malloc(strlen(new_list->items[i]) + strlen(tokens->items[loc])+ 2);
			sprintf(execFile,"%s/%s",new_list->items[i], tokens->items[loc]);

			args->items[0] = (char *) realloc(args->items[0], strlen(execFile) + 1);
			strcpy(args->items[0], execFile);
			//If there is a file that can be accessed = 0
			if(access(execFile,F_OK) == 0)
			{	//if found in path run command
				found = 1;
				int pid = fork();
				if(pid == 0)
				{	//in child
					if (outputFLG == 1)
					{
						close(STDOUT_FILENO);
						dup(fd);
						close(fd);
					}
					if (inputFLG == 1)
					{
						close(STDIN_FILENO);
						dup(fdin);
						close(fdin);
					}
					execv(execFile, args->items);
				}
				else
				{	//in parent(main)
					if(outputFLG == 1)
						close(fd);
					if (inputFLG == 1)
						close(fdin);
					waitpid(pid,NULL,0);
					free(execFile);
					free_tokens(args);
					//free(argsL); not sure if this is needed
				}
				free_tokens(new_list);
				free(path);
				break;
			}
		}
		if(found == 0 && *err == 0)
		{
			//Not found in path check if it's a file in one of path directories
			*err = 1;
			printf("Command not found %s\n", tokens->items[loc]);
			free_tokens(new_list);
			free(path);
		}
	}

	//*************************************************************************

	tokenlist *new_tokenlist(void)
	{
		tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
		tokens->size = 0;
		tokens->items = (char **) malloc(sizeof(char *));
		tokens->items[0] = NULL; /* make NULL terminated */
		return tokens;
	}

	void add_token(tokenlist *tokens, char *item)
	{
		int i = tokens->size;

		tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
		tokens->items[i] = (char *) malloc(strlen(item) + 1);
		tokens->items[i + 1] = NULL;
		strcpy(tokens->items[i], item);

		tokens->size += 1;
	}

	char *get_input(void)
	{
		char *buffer = NULL;
		int bufsize = 0;

		char line[5];
		while (fgets(line, 5, stdin) != NULL) {
			int addby = 0;
			char *newln = strchr(line, '\n');
			if (newln != NULL)
				addby = newln - line;
			else
				addby = 5 - 1;

			buffer = (char *) realloc(buffer, bufsize + addby);
			memcpy(&buffer[bufsize], line, addby);
			bufsize += addby;

			if (newln != NULL)
				break;
		}

		buffer = (char *) realloc(buffer, bufsize + 1);
		buffer[bufsize] = 0;

		return buffer;
	}

	tokenlist *get_tokens(char *input)
	{
		char *buf = (char *) malloc(strlen(input) + 1);
		strcpy(buf, input);

		tokenlist *tokens = new_tokenlist();

		char *tok = strtok(buf, " ");
		while (tok != NULL) {
			add_token(tokens, tok);
			tok = strtok(NULL, " ");
		}

		free(buf);
		return tokens;
	}

	void free_tokens(tokenlist *tokens)
	{
		for (int i = 0; i < tokens->size; i++)
			free(tokens->items[i]);

		free(tokens);
	}
	//*************************************************************************

	void replaceTokens (tokenlist *tokens, int *error, tokenlist *paths)
	{
		char *temp = NULL;
		char *buffer = NULL;
		char *tempItem = NULL;
		char *tempPath = NULL;
		int found = 0;
	  for (int i = 0; i < tokens->size; i++)
	  {
			found = 0;
			tempItem = (char *) malloc(strlen(tokens->items[i]));
			strcpy(tempItem, tokens->items[i]);
	    if (memchr(tempItem, '$', 1) != NULL)
	    {
				//Removing the '$' from the token
				temp = (char *) malloc(strlen(tempItem) - 1);
				strncpy(temp, (tempItem + 1), strlen(tempItem));
				//Getting the enviroment variable
				if (getenv(temp) != NULL)
				{
					buffer = (char *) malloc(strlen(getenv(temp)));
					strcpy(buffer, getenv(temp));

					//changing the space of the items and inputting the new string
					tokens->items[i] = (char *) realloc(tokens->items[i], strlen(buffer) + 1);
					strcpy(tokens->items[i], buffer);
					free(buffer);
				}
				else
				{
					printf("%s: Undefined Variable\n", temp);
					*error =  1;
					free(temp);
					free(buffer);
					free(tempItem);

					break;
				}
						free(temp);
	    }
			else if(memchr(tempItem, '~', 1) != NULL)
			{
				//Getting the home variable
				temp = (char *) malloc(strlen(getenv("HOME")) + 1);
				strcpy(temp, getenv("HOME"));

				//Getting the home and the concatintated string after
				buffer = (char *) malloc(strlen(getenv("HOME")) + strlen(tempItem));
				strcpy(buffer, temp);
				strncat(buffer, tempItem + 1, strlen(tempItem));

				//reallocting the tokens->items and copying the buffer into it
				tokens->items[i] = (char *) realloc(tokens->items[i], strlen(buffer) + 1);
				strcpy(tokens->items[i], buffer);
				free(buffer);
				free(temp);
			}
			/*
			else if( memchr(tempItem, '/', strlen(tempItem)) == NULL )
			{
				for (int j = 0; j < paths->size; j++)
				{
					//getting the path variable
					tempPath = (char *) malloc(strlen(paths->items[j]) + 1);
					strcpy(tempPath, paths->items[j]);

					//buffer is now ./Path/token->item[i]
					buffer = (char *) malloc(strlen(tempPath) + strlen(tokens->items[i]) + 2);
					strcpy(buffer, paths->items[j]);
					strcat(buffer, "/");
					strcat(buffer, tokens->items[i]);
					if (access(buffer, F_OK) == 0)
					{
						found = 1;
						break;
					}
						free(buffer);
						free(tempPath);
				}
				if (found == 1)
				{
					tokens->items[i] = (char *) realloc(tokens->items[i], strlen(buffer) + 1);
					strcpy(tokens->items[i], buffer);
					free(buffer);
					free(tempPath);
				}
				else
				{
					printf("Cannot find Command %s", tokens->items[i]);
					*error = 1;
				}
			}
			*/

			free(tempItem);

	  }

	}

	tokenlist * getPaths(void)
	{
		//getting path string into one
		char * buffer = (char *) malloc(strlen(getenv("PATH"))+ 1);
		strcpy(buffer, getenv("PATH"));

		//creating tok with strtok with delim ';'
		char * tok = strtok(buffer, ":");

		tokenlist *paths = new_tokenlist();

		while (tok != NULL)
		{
			add_token(paths, tok);
			tok = strtok(NULL, ":");
		}

		free(buffer);
		return paths;
	}

	int check_built(char * command)									//part 10
	{
		if(strcmp(command, "exit") == 0 || strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0)
			return 1;
		else
			return 0;
	}

	void built_in(tokenlist * token_ptr, int x)
	{
		if(strcmp(token_ptr->items[0], "exit") == 0)
		{
			exit_func(x);
		}
		if(strcmp(token_ptr->items[0], "cd") == 0)
		{
			change_directory(token_ptr);
		}
		if(strcmp(token_ptr->items[0], "echo") == 0)
		{
			echo(token_ptr);
		}

	}

	void exit_func(int size)
	{
	    printf("Commands executed : %d\n", size);
	    exit(0);
	}

	void change_directory(tokenlist * tokenptr)
	{
	    char* newdirectory = (char*) calloc(100, 100);

	    if(tokenptr->size == 1)             //user enters cd
	    {
	        char* homedir = getenv("HOME");
	        chdir(homedir);
	        setenv("PWD", homedir, 1);
	    }

	    else if(tokenptr->size > 2)         //user enters too many args
	    {
	        printf("Error. Too many arguments.\n");
	    }

	    /*
	    else
	    {
	        char* newdirectory = (char*) calloc(100, 100);

	    }*/
	}

	void echo(tokenlist * token_ptr)
	{
		char * tempvar;
		char * permvar;
		char * var;
		for (int i = 0; i < token_ptr->size; i++)
		{
			if(token_ptr->items[i][0] == '$')
			{
				tempvar = strtok(token_ptr->items[i], "$");
				permvar = getenv(tempvar);

				if(permvar != NULL)
				{
					printf("%s", permvar);
				}
			}
			else
			{
				printf("%s", token_ptr->items[i]);
			}
		}
		printf("\n");
	}

	void output_redirect(char* file)                     //starting part 7
	{
	    int fd = open(file, O_WRONLY | O_CREAT);
	    close(STDOUT_FILENO);
	    dup(fd);
	    close(fd);
	}

	void input_redirect(char* file)
	{
	    int fd = open(file, O_RDWR);
	    if(path_resolution(file) == 0)
	    {
	        printf("Error. File does not exist\n");
	    }

	    close(STDIN_FILENO);
	    dup(fd);
	    close(fd);
	}

	int pipe_func(tokenlist * token_ptr)
	{
		int num_pipes = 0;

		for (int i = 0; i < token_ptr->size; i++)
		{
			if(token_ptr->items[i][0] == '|')
				num_pipes++;
		}
		if(num_pipes == 1)
		{

		}
	}

	int path_resolution(char* file)
	{

	    FILE* file1;
	    file1 = fopen(file, "r");
	    if (file1 != NULL)
	    {
	        fclose(file1);
	        return 1;
	    }
	    return 0;
	}
