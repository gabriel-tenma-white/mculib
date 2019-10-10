#pragma once

static inline int ndigits(int64_t i) {
	int n=0;
	while(i!=0) {
		n++;
		i/=10;
	}
	return (n==0)?1:n;
}
static inline int itoa1(int64_t i, char* b) {
	static char const digit[] = "0123456789";
	char* p = b;
	if (i < 0) {
		*p++ = '-';
		i = -i;
	}
	p += ndigits(i);
	*p = '\0';
	int l = p - b;
	do { //Move back, inserting digits as u go
		*--p = digit[i % 10];
		i = i / 10;
	} while (i);
	return l;
}
