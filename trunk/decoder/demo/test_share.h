#ifndef TEST_SHARE_H
#define TEST_SHARE_H

typedef struct calc_share
{
	char *last_string;
	int last_string_len;

	int last_frag;
}calc_share_t;

struct yy_buffer_state;
struct yy_buffer_state* calc_scan_stream(const char *new_buf, size_t new_buf_len,
	const char *last_buf, size_t last_buf_len, void *scanner);
int calcerror(const char *format,...);
#endif
