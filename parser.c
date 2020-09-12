#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct {
	int size;
	char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void pathSearch(tokenlist *tokens, int loc, int *err);
void envVar (tokenlist * tokens, int *error);

int path_resolution(char* file);

void output_redirect(char* file);
void input_redirect(char* file);

void exit_func(int);
void change_directory(tokenlist * tknptr);

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

		if (strncmp(input, "exit", 4) == 0)
		{
			printf("Commands executed: ");
    }

		tokenlist *tokens = get_tokens(input);
		envVar(tokens, error);

		if (*error == 0)
		{
			for (int i = 0; i < tokens->size; i++)
			{
				printf("token %d: (%s)\n", i, tokens->items[i]);

				if(memchr(tokens->items[i], '/', strlen(tokens->items[i])) == NULL && i == 0)
				{
					pathSearch(tokens, i, error);
					if (*error != 0)
						break;
						break;
				}
			}
		}

		free(input);
		free_tokens(tokens);   //Memory allocation error caused from changing $ token, ~ seems to not have issue
	}

	return 0;
}

//Part 5-6: Path Search and ls execv
void pathSearch(tokenlist *tokens, int loc, int *err)
{
	char *argsL[tokens->size + 1];
	for (int i = 1, j = 1; i <= tokens->size; i++)
	{
		if(i != tokens->size)
		{
			argsL[j] = (char *) malloc(strlen(tokens->items[i]) + 1);
			strcpy(argsL[j], tokens->items[i]);
			j++;
		}
		else
			argsL[j] = NULL;
	}
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
	char* execFile;
	for(int i = 0; i < new_list->size; i++)
	{
		execFile = (char*)malloc(strlen(new_list->items[i]) + strlen(tokens->items[loc])+ 2);
		sprintf(execFile,"%s/%s",new_list->items[i], tokens->items[loc]);

		argsL[0] = (char *) malloc(strlen(execFile) + 1);
		strcpy(argsL[0], execFile);
		//If there is a file that can be accessed = 0
		if(access(execFile,F_OK) == 0)
		{	//if found in path run command
			found = 1;
			int pid = fork();
			if(pid == 0)
			{	//in child
				execv(execFile,argsL);
			}
			else
			{	//in parent(main)
				waitpid(pid,NULL,0);
				free(execFile);
				for (int i = 0 ; i < tokens->size; i++)
					free(argsL[i]);
				//free(argsL); not sure if this is needed
			}
			free_tokens(new_list);
			free(path);
			break;
		}
	}
	if(found == 0)
	{
		//Not found in path check if it's a file in one of path directories
		*err = 1;
		printf("Commnad not found %s\n", tokens->items[loc]);
		free_tokens(new_list);
		free(path);
	}
}

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

void envVar (tokenlist *tokens, int *error)
{
	char *temp = NULL;
	char *buffer = NULL;
	char *tempItem = NULL;
  for (int i = 0; i < tokens->size; i++)
  {
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

		free(tempItem);
  }
}
/*
void exit_func(int size)                            //starting part 10
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

    else
    {
        char* newdirectory = (char*) calloc(100, 100);

    }
}

void output_redirect(char* file)                     //starting part 7
{
    int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    close(STDOUT_FILENO);
    dup(fd);
    close(fd);
}

void input_redirect(char* file)
{
    int fd = open(file, O_RDWR | S_IRUSR);
    if(path_resolution(file) == 0)
    {
        printf("Error. File does not exit\n");
    }

    close(STDIN_FILENO);
    dup(fd);
    close(fd);
}

int path_resolution(char* file)
{
    FILE* file1;
    file1 = fopen(file, "r");

    if (file != NULL)
    {
        fclose(file1);
        return 1;
    }

    return 0;
}
*/
