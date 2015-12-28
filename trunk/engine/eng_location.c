#include "eng_priv.h"

const ey_location_t default_location = {"default-location", 1, 1, 1, 1};

ey_location_t *ey_parser_location(ey_engine_t *eng, ENG_RULE_LTYPE *loc)
{
	static ey_location_t ret;
	ey_parser_t *parser = SLIST_FIRST(&eng->parser_stack);
	ret.filename = parser->filename;
	ret.first_line = loc->first_line;
	ret.first_column = loc->first_column;
	ret.last_line = loc->last_line;
	ret.last_column = loc->last_column;
	return &ret;
}

ey_location_t *ey_combine_parser_location(ey_engine_t *eng, ENG_RULE_LTYPE *start_loc, ENG_RULE_LTYPE *end_loc)
{
	static ey_location_t ret;
	ey_parser_t *parser = SLIST_FIRST(&eng->parser_stack);
	ret.filename = parser->filename;
	ret.first_line = start_loc->first_line;
	ret.first_column = start_loc->first_column;
	ret.last_line = end_loc->last_line;
	ret.last_column = end_loc->last_column;
	return &ret;
}

ey_location_t *ey_combine_location(ey_engine_t *eng, ey_location_t *start_loc, ey_location_t *end_loc)
{
	static ey_location_t ret;
	ret.filename = start_loc->filename;
	ret.first_line = start_loc->first_line;
	ret.first_column = start_loc->first_column;
	ret.last_line = end_loc->last_line;
	ret.last_column = end_loc->last_column;
	return &ret;
}
