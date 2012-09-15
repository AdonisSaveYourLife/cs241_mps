/* 
 * CS 241
 * The University of Illinois
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "libmapreduce.h"
#include "libdictionary.h"

#define CHARS_PER_INPUT 30000
#define INPUTS_NEEDED 10

static const char *long_key = "Longest_Word";

/* Takes input string and maps the longest
 * word to the key, long_key.
 */
void map(int fd, const char *data)
{
	char longest_word[100];
	char s[200];
	longest_word[0] = '\0';

	char word[100];
	word[0] = '\0';

	while (1)
	{
		int word_len 		 = 0;
		const char *word_end = data;

		/* Get each word... */
		while (1)
		{
			if ( ((*word_end == ' ' || *word_end == '\n') && word_len > 0) || *word_end == '\0' || word_len == 99)
				break;
			else if (!isalpha(*word_end))
				break;
			else{
				word[word_len++] = *word_end;
				word_end++;
			}
		}
		word[word_len] = '\0';
		if( strlen(word) > strlen(longest_word) )
		{
			strcpy(longest_word,word);
		}

		if (*word_end == '\0') { break; }

		data = word_end + 1;
	}

	int len = snprintf(s, 200, "%s: %s\n", long_key, longest_word);
	write(fd, s, len);

	close(fd);
}

/* Takes two words and reduces to the longer of the two
 */
const char *reduce(const char *value1, const char *value2)
{
	char *result;
	if( strlen(value1) > strlen(value2) )
	{
		asprintf(&result, "%s", value1);
	}
	else asprintf(&result,"%s",value2);

	return result;
}


int main()
{
	FILE *file = fopen("11.txt", "r");
	char s[1024];
	int i;

	char **values = malloc(INPUTS_NEEDED * sizeof(char *));
	int values_cur = 0;
	
	values[0] = malloc(CHARS_PER_INPUT + 1);
	values[0][0] = '\0';

	while (fgets(s, 1024, file) != NULL)
	{
		if (strlen(values[values_cur]) + strlen(s) < CHARS_PER_INPUT)
			strcat(values[values_cur], s);
		else
		{
			values_cur++;
			values[values_cur] = malloc(CHARS_PER_INPUT + 1);
			values[values_cur][0] = '\0';
			strcat(values[values_cur], s);
		}
	}

	values_cur++;
	values[values_cur] = NULL;
	
	fclose(file);

	mapreduce_t mr;
	mapreduce_init(&mr, map, reduce);

	mapreduce_map_all(&mr, (const char **)values);
	mapreduce_reduce_all(&mr);
	
	const char *result_longest = mapreduce_get_value(&mr, long_key);

	if (result_longest == NULL) { printf("Longest Word: Failure\n"); }
	else { printf("Longest Word: %s\n", result_longest); }

	mapreduce_destroy(&mr);

	for (i = 0; i < values_cur; i++)
		free(values[i]);
	free(values);

	return 0;
}
