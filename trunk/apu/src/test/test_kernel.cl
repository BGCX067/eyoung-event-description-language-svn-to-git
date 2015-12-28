typedef struct apu_share
{
	int flag;
	int length;
	int total;
	int id;
	char buf[512];
}apu_share_t;

__kernel void hello(__global apu_share_t *p)
{
	__local int index;
	__private volatile int flag = 0;
	while(1)
	{
		flag = p->flag;
		if(flag)
			break;
	}

	p->total = 0;
	for(index=0; index<p->length; index++)
	{
		p->total += (int)(p->buf[index]);
		p->buf[index] = 'A';
	}
	p->flag = 0;
	p->id = get_global_id(0);
//	p[0] = 0;
//	p[2] = 0;
//	p[3] = get_global_id(0);
}
