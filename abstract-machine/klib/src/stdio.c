#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)//没有定义使用native

uint32_t itoa(uint32_t n, char* str, uint32_t radix, int upper, int sign, int width, int precision, int fillzero)//整数转字符串(不限制是有符号还是无符号)
{
	char tempstr[128];//由于存在宽度的问题，后面可能调整
	int i = 0, rem, k = 0;
	if (radix == 10  && sign==1 && (n >> 31)) {//十进制有符号数且为负数
		n = ~n + 1;//取反加一求原码
		tempstr[i++] = '-';//字符串最前面设置负号且下标加一
	}
	do {
		rem = n % radix;
		tempstr[i++] = (rem >= 10) ? ((upper == 1) ? 'A' + rem - 10 : 'a' + rem - 10) : '0' + rem;
		n /= radix;
	} while (n);
	//下面要判断精度问题
	//当precision比当前数长度大时(注意是数，不包含负号的)，要补上高位0
	if (tempstr[0] == '-') {
		if (precision + 1 > i) while (i <= precision) tempstr[i++] = '0';
	}
	else {
		if (precision > i) while (i < precision) tempstr[i++] = '0';
	}
	tempstr[i] = '\0';
	//反序
	if (tempstr[0] == '-') k = 1;
	char temp;
	int j = i - 1;
	while (k < j) {
		temp = tempstr[k];
		tempstr[k] = tempstr[j];
		tempstr[j] = temp;
		++k;
		--j;
	}
	int ret = i;
	int tempww = width;
	if(tempww>i){//当width大于后面串长时,要补空格
		while(tempww>i){
			*str++ = fillzero==1? '0' : ' ';//判断是补0还是补上1
			tempww--;
		}
		ret = width;
	}
	strcpy(str, tempstr);
	return ret;
}

int printf(const char* fmt, ...) {
	char buffer[2048];//缓冲区
	va_list ap;
	va_start(ap, fmt);
	int ret = vsprintf(buffer, fmt, ap);
	va_end(ap);
	for (int i = 0; i < ret; ++i) putch(buffer[i]);
	return ret;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
	char* start = out;
	int width=0,precision=0,fillzero=0;//宽度和精度以及宽度是否是补0
	while (*fmt != '\0') {
    if (*fmt != '%') *out++ = *fmt;
		else {
      fmt++;
			if(*fmt == '0'){
				fillzero = 1;
				fmt++;
			}//宽度补0
      if(*fmt>='0'&&*fmt<='9'){//确定宽度
        int ww=0;
        while(*fmt>='0'&&*fmt<='9'){
					ww = ww*10+*fmt-'0';
          fmt++;
        }
        width = ww;
      }
      else if(*fmt=='*'){
				width = va_arg(ap,int);
				fmt++;
      }
      if(*fmt == '.'){//确定精度
				fillzero = 0;//后面有精度，宽度补空格
				fmt++;
        if(*fmt>='0' && *fmt<='9'){
					int pp=0;
          while(*fmt>='0' && *fmt<='9'){
						pp = pp*10+*fmt-'0';
            fmt++;
          }
          precision = pp;
        }
        else if(*fmt == '*') {
					precision = va_arg(ap,int);
					fmt++;
				}
      }
      switch (*fmt) {
       case 'd': out += itoa(va_arg(ap, int), out, 10, 0, 1, width, precision, fillzero); break;
       case 'i': out += itoa(va_arg(ap, int), out, 10, 0, 1, width, precision, fillzero); break;
       case 'u': out += itoa(va_arg(ap, uint32_t), out, 10, 0, 0, width, precision, fillzero); break;
       case 'o': out += itoa(va_arg(ap, uint32_t), out, 8, 0, 0, width, precision, fillzero); break;
       case 'x': out += itoa(va_arg(ap, uint32_t), out, 16, 0, 0, width, precision, fillzero); break;
       case 'X': out += itoa(va_arg(ap, uint32_t), out, 16, 1, 0, width, precision, fillzero); break;
       case 'c': *out++ = va_arg(ap,int); break;
       case '%': *out++ = va_arg(ap,int); break;
       case 's': {
         char* temp = va_arg(ap, char*);
         //注意宽度
         int len = strlen(temp);
         if(width>len){
           while(width>len) *out++ = ' ';//空格填充
         }
         strcpy(out, temp);
         out += strlen(out);
         break;
       }
      }
			//复位
			width = 0;
			precision = 0;
			fillzero = 0;
     }
     fmt++;
   }
   *out = '\0';
	return out - start;
}

int sprintf(char* out, const char* fmt, ...)
{
	char* start = out;
	va_list ap;
	va_start(ap, fmt);
	int width=0,precision=0,fillzero=0;//宽度和精度以及宽度是否是补0
	while (*fmt != '\0') {
		if (*fmt != '%') *out++ = *fmt;
		else {
			fmt++;
			if(*fmt=='0') {
				fillzero = 1;
				fmt++;
				}//宽度补0
			if(*fmt>='0'&&*fmt<='9'){//确定宽度
				int ww=0;
				while(*fmt>='0'&&*fmt<='9'){
					ww = ww*10+*fmt-'0';
					fmt++;
				}
				width = ww;
			}
			else if(*fmt=='*'){
				width = va_arg(ap,int);
				fmt++;
			}
			if(*fmt == '.'){//确定精度
				fillzero = 0;//当存在精度的时候，宽度补空格
				fmt++;
				if(*fmt>='0' && *fmt<='9'){
					int pp=0;
					while(*fmt>='0' && *fmt<='9'){
						pp = pp*10+*fmt-'0';
						fmt++;
					}
					precision = pp;
				}
				else if(*fmt == '*'){
					 precision = va_arg(ap,int);
					 fmt++;
				}
			}
			switch (*fmt) {
			case 'd': out += itoa(va_arg(ap, int), out, 10, 0, 1, width, precision, fillzero); break;
			case 'i': out += itoa(va_arg(ap, int), out, 10, 0, 1, width, precision, fillzero); break;
			case 'u': out += itoa(va_arg(ap, uint32_t), out, 10, 0, 0, width, precision, fillzero); break;
			case 'o': out += itoa(va_arg(ap, uint32_t), out, 8, 0, 0, width, precision, fillzero); break;
			case 'x': out += itoa(va_arg(ap, uint32_t), out, 16, 0, 0, width, precision, fillzero); break;
			case 'X': out += itoa(va_arg(ap, uint32_t), out, 16, 1, 0, width, precision, fillzero); break;
			case 'c': *out++ = va_arg(ap,int); break;
			case '%': *out++ = va_arg(ap,int); break;
			case 's': {
				char* temp = va_arg(ap, char*);
				//注意宽度
				int len = strlen(temp);
				if(width>len){
					while(width>len) *out++ = ' ';//空格填充
				}
				strcpy(out, temp);
				out += strlen(out);
				break;
			}
			}
			//记得复位
			width = 0;
			precision = 0;
			fillzero = 0;
		}
		fmt++;
	}
	*out = '\0';
	va_end(ap);
	return out - start;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
