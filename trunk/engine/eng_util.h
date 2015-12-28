#ifndef ENG_UTIL_H
#define ENG_UTIL_H 1

extern int debug_engine_parser;
extern int debug_engine_lexier;
extern int debug_engine_init;

extern int engine_parser_debug(const char *format, ...);
extern int engine_parser_error(const char *format, ...);

extern int engine_lexier_debug(const char *format, ...);
extern int engine_lexier_error(const char *format, ...);

extern int engine_init_debug(const char *format, ...);
extern int engine_init_error(const char *format, ...);
#endif
