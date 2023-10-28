#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int itoa(int n, char *str, uint32_t radix)
{
	assert(radix<=16 && radix>=2);
	uint32_t unum;
	int i=0,rem,k;
	if(radix==10&&n<0)//十进制数并且是负数
  {
      unum=(unsigned)-n;//将n的绝对值赋给unum
      str[i++]='-';//在字符串最前面设置为'-'号，并且索引加1
  }
  else unum=(unsigned)n;//若是n为正，直接赋值给unum
	//转换
	do{
		rem = unum%radix;
		str[i++] = (rem>=10)?'a'+rem-10:'0'+rem;
		unum /= radix;
	}while(unum);
	str[i]='\0';
	//调换顺序
	if(str[0] == '-') k=1;
	else k=0;
	char temp;
	int j=i-1;
	while(k<j){
		temp = str[k];
		str[k] = str[j];
		str[j] = temp;
		k++;
		j--;
	}
	return i;
}
int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char *s = out;
	while(*fmt!='\0'){
		if(*fmt != '%'){
			*out++ = *fmt;
		}
		else {
			fmt++;
			switch(*fmt){
				case 'd': out += itoa(va_arg(ap, int), out, 10); break;
				case 'i': out += itoa(va_arg(ap, int), out, 10); break;
				case '%': *out++ = *fmt; break;
				case 's': {
										char *temp = va_arg(ap, char*);
										strcpy(out, temp);
										out += strlen(out);
										break;
									}
			}
		}
		fmt++;
	}
	*out = '\0';
	va_end(ap);
	return out-s;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
