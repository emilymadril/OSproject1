#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int size;
	char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void envVar(tokenlist *tokens);
tokenlist * getPaths(void);

int main()
{
	while (1) {
		printf("%s@%s : %s> ", getenv("user"), getenv("machine"), getenv("pwd"));

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);


		tokenlist *tokens = get_tokens(input);
		tokenlist *paths = getPaths();
    envVar(tokens);
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		for (int i = 0; i < paths->size; i++){
			printf("path %d: (%s)\n", i, paths->items[i]);
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

tokenlist *new_tokenlist(void)
{
	tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = NULL;
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;

	tokens->items = (char **) realloc(tokens->items, (i + 1) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
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

void envVar (tokenlist *tokens)
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
		}

		free(tempItem);
		free(temp);

  }
}

tokenlist * getPaths(void)
{
	//getting path string into one
	char * buffer = (char *) malloc(strlen(getenv("PATH"))+ 1);
	strcpy(buffer, getenv("PATH"));

	//creating tok with strtok with delim ';'
	char * tok = strtok(buffer, ";");

	tokenlist *paths = new_tokenlist();

	while (tok != NULL)
	{
		add_token(paths, tok);
		tok = strtok(NULL, ";");
	}

	free(buffer);
	return paths;
}
