#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
		int len = 0;
		if(s!=NULL){
			while(*s++ != '\0')
					len ++;
		}
    return len;
}

char *strcpy(char *dst, const char *src) {
	char* ret = dst;
	while (*src!='\0') {
		*dst=*src;
		dst++;
		src++;
	}
	*dst='\0';
	return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char* ret = dst; 
	while (n && (*dst++ = *src++)) 
	{
		--n;
	}
	if (n) 
	{
		while (n)
		{
			*dst++ = '\0';
			n--;
		}
	}
	return ret;
}

char *strcat(char *dst, const char *src) {
	char *p = dst;
	while(*p++ != '\0'){;}
	while(*src != '\0'){
		*p = *src;
		p++;
		src++;
	}
	*p='\0';
	return dst;
}

int strcmp(const char *s1, const char *s2) {
	while(*s1 != '\0' && (*s1==*s2)) {
      s1++;
      s2++;
  }
	int t=*s1-*s2;
	if(t==0) return 0;
	else if(t<0) return -1;
	else return 1;
	return t;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	if(n==0) return 0;
	while(n-- && *s1!='\0' &&*s1==*s2){
		s1++;
		s2++;
	}
	int t=*s1-*s2;
	if(t==0) return 0;
	else if(t<0) return -1;
	else return 1;
	return t;
}

void *memset(void *s, int c, size_t n) {
	if(s==NULL || n<0){
		return NULL;
	}
	char *p=(char *)s;
	while(n--){
		*p++ = c;
	}
	*p='\0';
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
	for (int i = n; i >= 0; --i) {
		*((char*)dst + i) = *((char*)src + i);
	}
	return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for (int i = 0; i < n; ++i) {
		*((char*)out + i) = *((char*)in + i);
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	assert(s1!=NULL&&s2!=NULL&&n>0);
	const char *p=s1;
	const char *t=s2;
	while(n-- && *p==*t){
		p++;
		t++;
	}
	int ans=*p-*t;
	if(ans==0) return 0;
	else if(ans<0) return -1;
	else return 1;
	return ans;
}

#endif
