#include<stdio.h>
#include<stdlib.h>

#if !defined(CHECK_SUM)
#define CHECK_SUM
unsigned short checksum(unsigned int* data, int len);
#endif 

void main()
{
    int dataNum = 0;
    scanf("%d", &dataNum);

    //unsigned int hexData[dataNum];
    unsigned int *hexData = (unsigned int *)malloc(sizeof(unsigned int) * dataNum);
    for(int i = 0; i < dataNum; i++)
    {
        scanf("%x",&hexData[i]);
    }

    unsigned short sum = checksum(hexData, dataNum);
    printf("%04x\n",sum);
}

unsigned short checksum(unsigned int* data, int len)
{
	unsigned int chksum = 0;  //expand to 32bit
	for(int i = 0; i < len; i++)
	{
		chksum += data[i];
		unsigned short low = chksum & 0xffff;
		unsigned short high = chksum >> 16;
		chksum = low + high;
	}
		
	return ~(chksum & 0xffff);
}