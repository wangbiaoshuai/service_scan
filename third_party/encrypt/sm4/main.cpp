#include <unistd.h>

typedef int (*pSMS4_encrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);
typedef int (*pSMS4_decrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);

int main()
{
	void * handle = dlopen("", RTLD_NOW);

	pSMS4_encrypt encrypt = (pSMS4_encrypt)dlsym(handle, "");
	if(!encrypt)
	{
		char* perror = dlerror();
	}

	pSMS4_decrypt decrypt = (pSMS4_decrypt)dlsym(handle, "");
	if(!decrypt)
	{
		char* perror = dlerror();
	}

	return 0;

}
