#ifndef TEST_H
#define TEST_H

#define BUF_LEN 512
#define BUF_SET 10
typedef struct apu_share
{
	int flag;
	int length;
	int total;
	int id;
	char buf[BUF_LEN];
}apu_share_t;
#endif
