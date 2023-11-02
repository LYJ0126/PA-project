#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)//没有定义使用native

uint32_t itoa(uint32_t n, char* str, uint32_t radix, int upper, int sign)//整数转字符串(不限制是有符号还是无符号)
{
	assert(radix <= 16 && radix >= 2);
	int i = 0, rem, k = 0;
	if (radix == 10  && sign==1 && (n >> 31)) {//十进制有符号数且为负数
		n = ~n + 1;//取反加一求原码
		str[i++] = '-';//字符串最前面设置负号且下标加一
	}
	do {
		rem = n % radix;
		str[i++] = (rem >= 10) ? ((upper == 1) ? 'A' + rem - 10 : 'a' + rem - 10) : '0' + rem;
		n /= radix;
	} while (n);
	str[i] = '\0';
	//反序
	if (str[0] == '-') k = 1;
	char temp;
	int j = i - 1;
	while (k < j) {
		temp = str[k];
		str[k] = str[j];
		str[j] = temp;
		++k;
		--j;
	}
	return i;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char* out, const char* fmt, ...)
{
	char* start = out;
	va_list ap;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt != '%') *out++ = *fmt;
		else {
			fmt++;
			switch (*fmt) {
			case 'd': out += itoa(va_arg(ap, int), out, 10, 0, 1); break;
			case 'i': out += itoa(va_arg(ap, int), out, 10, 0, 1); break;
			case 'u': out += itoa(va_arg(ap, uint32_t), out, 10, 0, 0); break;
			case 'o': out += itoa(va_arg(ap, uint32_t), out, 8, 0, 0); break;
			case 'x': out += itoa(va_arg(ap, uint32_t), out, 16, 0, 0); break;
			case 'X': out += itoa(va_arg(ap, uint32_t), out, 16, 1, 0); break;
			case 'c': *out++ = va_arg(ap,int); break;
			case '%': *out++ = va_arg(ap,int); break;
			case 's': {
				char* temp = va_arg(ap, char*); break;
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
	return out - start;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
