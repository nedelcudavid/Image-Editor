#ifndef _PARSER_H
#define _PARSER_H

typedef struct parser_t {
	int argc;
	char **argv;
} parser_t;

#define PARSER_INIT(parser) ((void)memset((parser), 0, sizeof(parser_t)))

void parser_clear(parser_t *parser);

void parser_parse(parser_t *parser, char *command);

#endif
