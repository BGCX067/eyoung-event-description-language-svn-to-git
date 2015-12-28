#ifndef ENG_PARSER_H
#define ENG_PARSER_H 1

#include "ey_queue.h"
#include "eng_location.h"

#define MAX_ERROR_REASON 1024
typedef struct ey_parser
{
	SLIST_ENTRY(ey_parser) link;

	/*lexier*/
	void *lexier;
	char *filename;
	ey_location_t last_location;

	/*parser*/
	void *parser;
	unsigned int level;
	
	/*error handler*/
	ey_location_t error_location;
	char error_reason[MAX_ERROR_REASON];
}ey_parser_t;
typedef SLIST_HEAD(ey_parser_stack, ey_parser) ey_parser_stack_t;

struct ey_engine;
extern int ey_parse_file(struct ey_engine *eng, const char *filename);

extern int ey_parser_init(struct ey_engine *eng);
extern void ey_parser_finit(struct ey_engine *eng);
#endif
