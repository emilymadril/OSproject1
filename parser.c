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

int main()
{
	while (1) {
		int error = 0;
		
		printf("%s@%s : %s> ",getenv("USER"),getenv("MACHINE"),getenv("PWD"));	//Part 3: Prompt

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		
		for (int i = 0; i < tokens->size; i++) {
			if(strchr(tokens->items[i], '$') != NULL){			//Part 2: Environment Variables($)
				char *temp = NULL;
				char *buffer = NULL;
				char *tempItem = NULL;

				tempItem = (char *) malloc(strlen(tokens->items[i]));
				strcpy(tempItem, tokens->items[i]);

    			if (memchr(tempItem, '$', 1) != NULL){
					//Removing the '$' from the token
					temp = (char *) malloc(strlen(tempItem) - 1);
					strncpy(temp, (tempItem + 1), strlen(tempItem));

					//Getting the enviroment variable
					char* env_var = getenv(temp);
					if(env_var == NULL){
						//error message
						error = 1;
						break;
					}
					else{
						buffer = (char *) malloc(strlen(getenv(temp)));
						strcpy(buffer, env_var);

						//changing the space of the items and inputting the new string
						tokens->items[i] = (char *) realloc(tokens->items[i], strlen(buffer) + 1);
						strcpy(tokens->items[i], buffer);

						free(buffer);
					}
    			}
				free(tempItem);
				free(temp);
			}
			if(strchr(tokens->items[i], '~') != NULL){							//Part 4: Tilden Expansion(~)			
				char* hold = (char *) malloc(strlen(tokens->items[i]));
				strcpy(hold,tokens->items[i]);
				char* tild_excess = (char *) malloc(strlen(hold)-1);
				strncpy(tild_excess, (hold + 1),strlen(hold)-1);
				free(hold);

				char* new_token = (char *) malloc(strlen(getenv("HOME")) + strlen(tild_excess));
				free(tild_excess);
				/*
					if(tild excess isnt a valid path)
						throw error message
						error = 2
						break
				*/

				if(tild_excess == NULL)sprintf(new_token,"%s",getenv("HOME"));
				else
					sprintf(new_token,"%s%s",getenv("HOME"),tild_excess);	

				tokens->items[i] = (char *) realloc(tokens->items[i], strlen(new_token) + 1);
				strcpy(tokens->items[i], new_token);

				free(new_token);
			}	
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}
		if(error != 0)continue;

		free(input);
		free_tokens(tokens);   //Memory allocation error caused from changing $ token, ~ seems to not have issue
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
