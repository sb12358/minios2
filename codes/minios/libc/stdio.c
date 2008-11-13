#include <device.h>
#include <string.h>
#include <stdarg.h>

#define ZEROPAD 1		/* pad with zero */ 
#define SIGN 2			/* unsigned/signed long */ 
#define PLUS 4			/* show plus */ 
#define SPACE 8			/* space if plus */ 
#define LEFT 16			/* left justified */ 
#define SPECIAL 32		/* 0x */ 
#define SMALL 64		/* use 'abcdef' instead of 'ABCDEF' */

char* number(char *str, long number, int base ,int width, int flag)
{
	char buffer[16];
	char p=0;
	char *digits="0123456789ABCDEF";
	char sign=0;
	char padding;
	int i;
	unsigned long num = (unsigned long)number;

	if (flag & SMALL) digits="0123456789abcdef";
	if (flag & LEFT) flag &= ~ZEROPAD; 
	if (base<8 || base>16) 
		return 0; 
   
	padding = (flag & ZEROPAD) ? '0' : ' ' ; 
	if (flag & SIGN && number < 0){ 
		sign= '-'; 
		num = - number; 
	}else  if(flag & PLUS)
		sign='+';
	else if(flag & SPACE)
		sign=' ';

	if(num==0)
		buffer[p++]='0';
	else while(num)
	{
		buffer[p++]=digits[num % base];
		num /= base;
	}
	
	if(sign)
		buffer[p++]=sign;

	if(flag & SPECIAL)
	{
		buffer[p++]='x';
		buffer[p++]='0';
	}

	if(flag & LEFT)
	{
		for(i=p-1;i>=0;i--)
			*str++=buffer[i];
		if(width>p)
		{
			for(i=0;i<width-p;i++)
				*str++=padding;
		}
	}else
	{
		if(width>p)
		{
			for(i=0;i<width-p;i++)
				*str++=padding;
		}
		for(i=p-1;i>=0;i--)
			*str++=buffer[i];
	}
	return str;
}

int skip_atoi(const char **s)
{
	int i=0;
	while(**s>='0' && **s<='9')
		i = i*10 + *((*s)++) - '0';
	return i;
}

int vsprintf(char *buf, const char *fmt, va_list arglist)
{
	char *s;
	char *t;
	char *str = buf;
	char dec;
	int width;
	int len;
	int i;

	while(*fmt)
	{
		if(*fmt=='%')
		{
			char flags = 0;
			char typelen = 'i';											//Remark
repeat:
			fmt++;
			switch (*fmt)
			{
				case '-': flags |= LEFT; goto repeat; 
				case '+': flags |= PLUS; goto repeat; 
				case ' ': flags |= SPACE; goto repeat; 
				case '#': flags |= SPECIAL; goto repeat; 
				case '0': flags |= ZEROPAD; goto repeat; 
			}

			width = skip_atoi(&fmt);
			
			if (*fmt=='h' || *fmt=='l')
			{
				typelen=*fmt;
				fmt++;
			}

			if (*fmt=='c')
				*buf++ = (unsigned char) va_arg(arglist, int);//Remark
			else if (*fmt=='s')
			{
				s= va_arg(arglist, char *);
				t=s;
				while(*t)
					t++;
				len = t-s;
				if (!(flags & LEFT)) 
				while (len < width--) 
					*buf++ = ' ';
				for (i = 0; i < len; ++i) 
					*buf++ = *s++; 
				while (len < width--) 
					*buf++ = ' ';
			}else
			{
				switch (*fmt)
				{
					case 'd':
					case 'i':
						flags |= SIGN;
					case 'u':
						dec=10;
						break;
					case 'x':
						flags |= SMALL;
					case 'X':
						dec=16;
						break;
					case 'p':
						flags |= SMALL;
					case 'P':
						dec=16;
						width=8;								//Remark
						flags |= ZEROPAD; 
						break;
					case 'o':
						dec=8;
						break;
				}
				if(typelen=='h')            //Remark
					buf = number(buf, va_arg(arglist, int), dec, width, flags);
				else if(typelen=='i')
					buf = number(buf, va_arg(arglist, int), dec, width, flags);
				else
					buf = number(buf, va_arg(arglist, long), dec, width, flags);
			}
			fmt++;
		}else
		{
			*buf++ = *fmt++;
		}
	}
	*buf=0;
	return buf-str;
}

int sprintf( char *buffer, const char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	return vsprintf(buffer, fmt, arglist);
}

extern pvoid stdout;

int printf(char *fmt, ...)
{
	int len=0;
	char buffer[256];
	va_list arglist;
	va_start(arglist, fmt);
	len=vsprintf(buffer, fmt, arglist);
	write(stdout, buffer, len);
	return len;
}

int puts(char *str)
{
	int len=strlen(str);
	write(stdout, str, len);
	return len;
}

extern pvoid stdin;

char* gets(char*buf)
{
	int len=0;
	uint8 ch;

	read(stdin, &ch, 1);
	while(ch!='\n')
	{
		if(ch=='\b')
		{
			if(len>0)
			{
				len--;
				write(stdout, &ch, 1);
			}
			read(stdin, &ch, 1);
		}else
		{
			buf[len++]=ch;
			write(stdout, &ch, 1);
			read(stdin, &ch, 1);
		}
	}
	buf[len++]=ch;
	buf[len]=0;
	write(stdout, &ch, 1);
	return buf;
}