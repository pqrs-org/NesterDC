/* KallistiOS 1.1.8

   atoi.c
   (c)2001 Vincent Penne

   atoi.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

int atoi(const char * s) {
	int m, v;

	v = 0;

	if (*s == '-') {
		m = -1;
		s++;
	} else {
		m = 1;
		if (*s == '+')
			s++;
	}

	while (*s >= '0' && *s <= '9') {
		v = v*10 + *s-'0';
		s++;
	}

	return m * v;
}

