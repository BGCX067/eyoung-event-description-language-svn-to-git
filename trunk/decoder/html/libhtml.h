#ifndef LIBHTML_H
#define LIBHTML_H 1

#define XSS_SENSITIVE_LOW		1
#define XSS_SENSITIVE_MIDDLE	2
#define XSS_SENSITIVE_HIGH		3
extern int xss_injection_check(const char *buf, int buf_len, int sensitive);

extern int parse_html_file(const char *filename);

extern int parse_html_frags(char **frags, int frags_num);

#ifndef HTML_DEBUG
extern int decoder_html_file_reg();
extern void decoder_html_file_unreg();
#endif

#endif
