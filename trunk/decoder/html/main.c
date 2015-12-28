#ifdef HTML_DEBUG
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libhtml.h"

extern void html_mem_init();
extern void html_mem_finit();

static void show_help()
{
	fprintf(stderr, "-t: 1-xss, 2-file, 3-frags\n");
	fprintf(stderr, "-f: filename\n");
	fprintf(stderr, "-r: 0-failse, 1-true\n");
	fprintf(stderr, "-s: 1-low, 2-middle, 3-high\n");
}

static int parse_cmdline(int argc, char *argv[], char **filename, int *type, int *expect, int *sensitive)
{
	int opt = 0;

	while((opt = getopt(argc, argv, "s:t:f:r:h")) != -1)
	{
		switch(opt)
		{
		case 'f':
			*filename = strdup(optarg);
			break;
		case 't':
			*type = atoi(optarg);
			break;
		case 'r':
			*expect = atoi(optarg);
			break;
		case 's':
			*sensitive = atoi(optarg);
			break;
		case 'h':
			show_help();
			exit(0);
		default:
			fprintf(stderr, "Error: bad parameter\n");
			show_help();
			return -1;
		}
	}

	if(!*filename)
	{
		fprintf(stderr, "Error: filename not found\n");
		return -1;
	}

	if(*type!=1 && *type!=2 && *type!=3)
	{
		fprintf(stderr, "Error: bad type\n");
		return -1;
	}

	if(*expect!=0 && *expect!=1)
	{
		fprintf(stderr, "Error: bad expect value\n");
		return -1;
	}

	if(*type==1 && *sensitive!=1 && *sensitive!=2 && *sensitive!=3)
	{
		fprintf(stderr, "Error: bad sensitive parameter\n");
		return -1;
	}

	return 0;
}

static int check_file(const char *filename, int expect)
{
	int tmp = parse_html_file(filename);
	if(!tmp != expect)
		fprintf(stderr, "testcase %s FAILED\n", filename);
	else
		fprintf(stderr, "testcase %s OK\n", filename);
	return 0;
}

static int check_xss(const char *filename, int expect, int sensitive)
{
	char xss_buf[64*1024];
	int xss_len=0;
	FILE *fp = NULL;

	fp = fopen(filename, "r");
	if(!fp)
	{
		fprintf(stderr, "Error: cannot open file %s\n", filename);
		goto ret;
	}

	xss_len = fread(xss_buf, 1, sizeof(xss_buf), fp);
	if(!feof(fp))
	{
		fprintf(stderr, "Error: too long xss buffer\n");
		goto ret;
	}

	if(ferror(fp))
	{
		fprintf(stderr, "Error: error while read file %s\n", filename);
		goto ret;
	}

	if(!expect == !xss_injection_check(xss_buf, xss_len, sensitive))
		fprintf(stderr, "testcase %s OK\n", filename);
	else
		fprintf(stderr, "testcase %s FAILED\n", filename);

ret:
	if(fp)
		fclose(fp);
	return 0;
}

static int check_frag(const char *filename, int expect)
{
	FILE *fp=NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int lines = 0;
	char **frags = NULL;

	fp = fopen(filename, "r");
	if(!fp)
	{
		fprintf(stderr, "Error: open file %s failed\n", filename);
		goto ret;
	}

	while((read = getline(&line, &len, fp)) != -1)
		lines++;

	if(!lines)
	{
		fprintf(stderr, "Error: null file %s\n", filename);
		goto ret;
	}
	
	frags = (char**)malloc(lines * sizeof(char*));
	if(!frags)
	{
		fprintf(stderr, "Error: malloc frags failed\n");
		goto ret;
	}

	fseek(fp, 0L, SEEK_SET);
	lines = 0;
	while((read = getline(&line, &len, fp)) != -1)
	{
		frags[lines] = strdup(line);
		if(!frags[lines])
		{
			fprintf(stderr, "Error: copy line %d failed\n", lines);
			goto ret;
		}
		if(read > 0)
			frags[lines][read-1] = '\0';
		lines++;
	}
	
	int tmp = parse_html_frags(frags, lines);
	if(!tmp != expect)
		fprintf(stderr, "testcase %s FAILED\n", filename);
	else
		fprintf(stderr, "testcase %s OK\n", filename);

ret:
	if(frags)
	{
		for(tmp=0; tmp<lines; tmp++)
		{
			if(frags[tmp])
				free(frags[tmp]);
		}
		free(frags);
	}
	if(line)
		free(line);
	if(fp)
		fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	char *filename=NULL;
	int type = -1;
	int expect = -1;
	int sensitive = -1;
	
	html_mem_init();
	if(parse_cmdline(argc, argv, &filename, &type, &expect, &sensitive))
		return -1;
	
	fprintf(stderr, "FileName: %s\n", filename);
	switch(type)
	{
	case 1:
		check_xss(filename, expect, sensitive);
		break;
	case 2:
		check_file(filename, expect);
		break;
	case 3:
		check_frag(filename, expect);
		break;
	default:
		break;
	}
	if(filename)
		free(filename);
	html_mem_finit();
	return 0;
}
#endif
