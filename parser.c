#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DELIMS "\n\t\x20"

void parser_clear(parser_t *parser)
{
	if (!parser || !parser->argc || !parser->argv)
		return;

	// Elibereaza memoria pentru stocarea parametrilor comenzii anterioare
	for (int i = 0; i < parser->argc; i++)
		free(parser->argv[i]);
	free(parser->argv);

	// Seteaza argumentele la 0/null
	PARSER_INIT(parser);
}

// Fa o copie alocata dinamic a stringului pasat ca argument
static char *__duplicate(const char *__s)
{
	char *new_str = (char *)malloc(strlen(__s) + 1);
	assert(new_str);
	return strcpy(new_str, __s);
}

// Descompune comanda in parametri si plaseaza-i in parser
void parser_parse(parser_t *parser, char *command)
{
	parser_clear(parser);

	// Primul token, numele comenzii
	char *token = strtok(command, DELIMS);
	if (!token)
		return;

	// Aloca memorie pentru primul parametru (comanda)
	parser->argv = (char **)calloc(1, sizeof(char *));
	assert(parser->argv);
	parser->argv[parser->argc++] = __duplicate(token);

	while ((token = strtok(NULL, DELIMS)) != NULL) {
		// Aloca spatiu pentru inca un string/parametru

		parser->argv = (char **)realloc(parser->argv, ++parser->argc
			* sizeof(char *));
		parser->argv[parser->argc - 1] = __duplicate(token);
	}
}
