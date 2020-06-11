#ifndef _MII_H_
#define _MII_H_
#define DAT_MIN_SIZE 0x1F1E0
#define MII_FILE_SIZE 0x4A
#define RKG_MII_DATA_OFFSET 0x3C
#define RKG_MIN_SIZE 0x88
#define MAX_MII_NUM 100
#define MII_NAME_LENGTH 10

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

typedef struct {
	unsigned char rawData[MII_FILE_SIZE];
    char name[MII_NAME_LENGTH];
	unsigned int month;
	unsigned int day;
	unsigned int favColor;
} mii;

int installMii(char*,char*);
long getFileSize(int fd);
int isfsGetFileSize(int);
int miiRawDataCheck(unsigned char*);
int miiFileWrite(mii*,int,char*);
void*allocate_memory(unsigned int);
void getMiiInfo(mii*);
void allGetMiiInfo(mii*,int);
unsigned short getCrc(unsigned char*,int);

#endif //_MII_H_