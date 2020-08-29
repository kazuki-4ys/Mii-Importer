#include "mii.hpp"

unsigned short getCrc(unsigned char*buf,int size){
    unsigned int crc = 0x0000;
    int byteIndex,bitIndex,counter;
    for (byteIndex = 0; byteIndex < size; byteIndex++) {
        for (bitIndex = 7; bitIndex >= 0; bitIndex--) {
            crc = (((crc << 1) | ((buf[byteIndex] >> bitIndex) & 0x1)) ^
            (((crc & 0x8000) != 0) ? 0x1021 : 0)); 
        }
    }
    for (counter = 16; counter > 0; counter--) {
        crc = ((crc << 1) ^ (((crc & 0x8000) != 0) ? 0x1021 : 0));
    }
    return (unsigned short)(crc & 0xFFFF);
}

int installMii(const char *path){
    const char TARGET_PATH[] = "/shared2/menu/FaceLib/RFL_DB.dat";
    const char DAT_MAGIC[] = "RNOD";
	const char RKG_MAGIC[] = "RKGD";
	int nandFd,fd,fileSize,miiNum = 0;
	long miiFileSize;
	char headBuf[5] = {};
	unsigned short crc;
	unsigned char *DatFileBuf; //RFL_DB.datを格納するバッファ
    nandFd = ISFS_Open(TARGET_PATH,ISFS_OPEN_RW);
	if(nandFd < 1){
		return -1;
	}
	fileSize = isfsGetFileSize(nandFd);
	if(fileSize < DAT_MIN_SIZE){
        ISFS_Close(nandFd);
		return -1;
	}
	DatFileBuf = (unsigned char*)allocate_memory(fileSize);//32バイト境界に載せる必要があるため、mallocは使用不可
    if(DatFileBuf == NULL){
        ISFS_Close(nandFd);
        printf("Error:allocate_memory\n");
        return -1;
    }
    ISFS_Read(nandFd,DatFileBuf,fileSize);
	memcpy(headBuf,DatFileBuf,4);
	if(strcmp(headBuf,DAT_MAGIC) != 0){
        printf("Error:magic doesn't match\n");
		free(DatFileBuf);
		ISFS_Close(nandFd);
		return -1;
	}
	while(1){
		if(miiNum >= MAX_MII_NUM){
			printf("Error:Mii is full\n");
			free(DatFileBuf);
			ISFS_Close(nandFd);
			return -1;
		}
        if(miiRawDataCheck(DatFileBuf + 4 + MII_FILE_SIZE * miiNum) != 0)break;
		miiNum++;
	}
    fd = open(path,O_RDONLY);
	if(fd < 0){
		printf("Error:open\n");
		free(DatFileBuf);
        ISFS_Close(nandFd);
		return -1;
	}
	miiFileSize = getFileSize(fd);
	if(miiFileSize != MII_FILE_SIZE && miiFileSize < RKG_MIN_SIZE){
        if(miiFileSize > 0)printf("Error:Invalid file\n");
        free(DatFileBuf);
		ISFS_Close(nandFd);
		close(fd);
		return -1;
	}
	FILE *f = fdopen(fd,"rb");
	if(!f){
		printf("Error:fdopen\n");
		ISFS_Close(nandFd);
		close(fd);
		free(DatFileBuf);
		return -1;
	}
	if(miiFileSize != MII_FILE_SIZE){
        fread(headBuf,sizeof(char),4,f);
		if(strcmp(headBuf,RKG_MAGIC) != 0){
			printf("Error:Invalid file\n");
			free(DatFileBuf);
		    ISFS_Close(nandFd);
		    close(fd);
			return -1;
		}
		fseek(f,RKG_MII_DATA_OFFSET,SEEK_SET);
	}
	fread(DatFileBuf + 4 + MII_FILE_SIZE * miiNum,sizeof(unsigned char),MII_FILE_SIZE,f);
	fclose(f);
	crc = getCrc(DatFileBuf,DAT_MIN_SIZE - 2);
	memcpy(DatFileBuf + DAT_MIN_SIZE - 2,&crc,2);
	if(ISFS_Seek(nandFd,0,SEEK_SET) != ISFS_OK){
        printf("Error:ISFS_Seek\n");
		ISFS_Close(nandFd);
        free(DatFileBuf);
		return -1;
	}
    ISFS_Write(nandFd,DatFileBuf,fileSize);
	free(DatFileBuf);
	ISFS_Close(nandFd);
	printf("Success!\n");
	return 0;
}

long getFileSize(int fd){
    struct stat stbuf;
	if(fstat(fd,&stbuf) == -1){
		printf("Error:fstat\n");
		close(fd);
        return -1L;
	}
	return stbuf.st_size;
}

int isfsGetFileSize(int fd){
	int error;
	if(fd < 1)return 0;
    fstats stats ATTRIBUTE_ALIGN(32);//ATTRIBUTE_ALIGN(32)で32バイト境界に載せる
	error = ISFS_GetFileStats(fd,&stats);
	if(error >= 0){
        return stats.file_length;
	}
	printf("Error:ISFS_GetFileStats (%d)\n",error);
	return 0;
}

int miiRawDataCheck(unsigned char*src){
    int i;
	for(i = 0;i < MII_FILE_SIZE;i++){
		if(src[i] != 0)return 0;
	}
	return -1;
}

void*allocate_memory(unsigned int size){
    void*buf = memalign(32,(size+31)&(~31));
	memset(buf,0,(size+31)&(~31));
	return buf;
}