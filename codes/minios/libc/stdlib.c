int abs(int a)
{
	if(a<0)
		return -a;
	else
		return a;
}

int atoi(char *str)
{
	int negsign=0;
	int ret=0;
	if(str==0)
		return 0;

	while(*str==' ')
		str++;
	
	if(*str=='-'){
		negsign=1;
		str++;
		while(*str==' ')
			str++;
	}else if(*str=='+')
	{
		str++;
		while(*str==' ')
			str++;
	}

	while(*str>='0' && *str<='9')
	{
		int a=*str-'0';
		ret*=10;
		ret+=a;
		str++;
	}

	if(negsign)
		return -ret;
	else
		return ret;
}

long atol(char *str)
{
	return atoi(str);
}

unsigned int stdlib_seed=0x12345678;

int rand()
{
    return((( stdlib_seed = stdlib_seed * 214013L + 2531011L) >> 16) & 0x7fff );
}

void srand(unsigned int seed)
{
	stdlib_seed = seed;
}
