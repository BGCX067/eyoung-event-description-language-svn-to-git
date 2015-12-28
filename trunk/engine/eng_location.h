#ifndef ENG_LOCATION_H
#define ENG_LOCATION_H 1

typedef struct ey_location
{
	char *filename;
	int first_line;
	int first_column;
	int last_line;
	int last_column;
}ey_location_t;

struct ey_engine;
struct ENG_RULE_LTYPE;
extern ey_location_t *ey_parser_location(struct ey_engine *eng, struct ENG_RULE_LTYPE *loc);
extern ey_location_t *ey_combine_parser_location(struct ey_engine *eng, struct ENG_RULE_LTYPE *start_loc, struct ENG_RULE_LTYPE *end_loc);
extern ey_location_t *ey_combine_location(struct ey_engine *eng, ey_location_t *start_loc, ey_location_t *end_loc);

#define print_location(loc) (loc)->filename,(loc)->first_line,(loc)->first_column,(loc)->last_line,(loc)->last_column

extern const ey_location_t default_location;
#endif
