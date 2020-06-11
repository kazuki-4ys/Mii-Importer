#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include "mii.h"

#define SHOW_FILE_NUM 15

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int sdInitialize = 0;
int isfsInitialize = 0;

char*fileNameList[MAX_MII_NUM] ={};

int SD_Initialize(){
	int ret = fatMountSimple("sd", &__io_wiisd);
	if(!ret) return ret;
	return 1;
}

void SD_Deinitialize(){
	fatUnmount("sd:/");
    __io_wiisd.shutdown();
}

void free_fileNameList(){
	int i = 0;
	while(1){
		if(!fileNameList[i])break;
		free(fileNameList[i]);
        i++;
	}
	return;
}

void printFileList(int index,int max){
    int page = index / SHOW_FILE_NUM,i;
	int curIndex;
	for(i = 0;i < SHOW_FILE_NUM;i++){
		curIndex = i + page * SHOW_FILE_NUM;
        if(curIndex >= max)break;
		if(curIndex == index){
            printf("-->> ");
		}else{
			printf("     ");
		}
		printf(fileNameList[curIndex]);
		printf("\n");
	}
	return;
}

void updateFileNameList(int index,int fileNum){
	int i;
    printf("\x1b[7;0H");
	for(i = 0;i < SHOW_FILE_NUM;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[7;0H");
	printFileList(index,fileNum);
	printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
	for(i = 0;i < 2;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
	printf("A / Install Mii | UP DOWN / Select Mii\n");
	printf("HOME / Exit\n");
}

void appExit(){
    if(sdInitialize){
        SD_Deinitialize();
	}
	if(isfsInitialize){
        ISFS_Deinitialize();
	}
	free_fileNameList();
	exit(0);
}

int checkExt(char *s){
    char *ptr = strlen(s) + s - 1;
	while(1){
		if(*ptr == '.' || ptr <= s)break;
		ptr--;
	}
	if(!strcmp(ptr,".mii")){
        return 1;
	}else if(!strcmp(ptr,".MII")){
        return 1;
	}else if(!strcmp(ptr,".miigx")){
        return 1;
	}else if(!strcmp(ptr,".MIIGX")){
        return 1;
	}else if(!strcmp(ptr,".mae")){
        return 1;
	}else if(!strcmp(ptr,".MAE")){
        return 1;
	}else if(!strcmp(ptr,".rkg")){
        return 1;
	}else if(!strcmp(ptr,".RKG")){
        return 1;
	}
	return 0;
}

int getFileNameList(char *dir,char**dest){
	int nameLength,fileNum = 0;
	struct dirent *d;
    DIR *pdir = opendir(dir);
	if(!pdir){
		printf("Error:%s doesn't found\n",dir);
		return -1;
	}
	while(1){
		d = readdir(pdir);
		if(!d)break;
        if(!checkExt(d->d_name))continue;
		nameLength = strlen(d->d_name);
        dest[fileNum] = calloc(nameLength + 1,sizeof(char));
		if(!dest[fileNum]){
            printf("Error:calloc\n");
			closedir(pdir);
			return -1;
		}
		memcpy(dest[fileNum],d->d_name,nameLength);
		fileNum++;
		if(fileNum >= MAX_MII_NUM)break;
	}
	closedir(pdir);
	return fileNum;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
    int index = 0,fileNum = -1,i;
	char miiDir[] = "sd:/MIIs";
	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	printf("\x1b[2;0H");	
	printf("+---------------------+\n");
	printf("|  Mii Importer v1.0  |\n");
    printf("| developed by Kazuki |\n");
    printf("+---------------------+\n");
    printf("[*] Initializing ISFS subsystem...");
	if(ISFS_Initialize() == ISFS_OK){
        printf(" OK!\n");
		isfsInitialize = 1;
	}else{
		printf(" Error!\n");
		goto error;
	}
	printf("[*] Mounting SD Card...");
    if (SD_Initialize()){
	    printf(" OK!\n");
		sdInitialize = 1;
		fileNum = getFileNameList(miiDir,fileNameList);
		if(fileNum <= 0){
		    if(fileNum == 0){
			    printf("%s is empty\n",miiDir);
		    }
			goto error;
		}
	}else{
        printf(" Error!\n");
	}
	error:
    if(fileNum > 0){
    printf("\x1b[6;0H");
        for(i = 0;i < 12;i++){
            printf("                                        \n");
	    }
	    printf("\x1b[6;0H");
	    printf("----------------------------------------------------------------------\n");
		printf("\x1b[%d;0H",SHOW_FILE_NUM + 7);
		printf("----------------------------------------------------------------------\n");
		printf("\x1b[7;0H");
		printFileList(index,fileNum);
		printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
		printf("A / Install Mii | UP DOWN / Select Mii\n");
		printf("HOME / Exit\n");
        printf("\x1b[6;0H");
	}else{
	    printf("\nPress HOME to exit\n");
	}
	while(1) {//メインループ

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME )appExit();
		if(fileNum > 0){
            if ( pressed & WPAD_BUTTON_UP ){
                if(index > 0){
                    index--;
					updateFileNameList(index,fileNum);
				}
			}else if(pressed & WPAD_BUTTON_DOWN){
                if(index < fileNum - 1){
                    index++;
					updateFileNameList(index,fileNum);
				}
			}else if(pressed & WPAD_BUTTON_A){
                printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
	            for(i = 0;i < 2;i++){
                    printf("                                        \n");
	            }
	            printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
				printf("Installing Mii...\n");
				installMii(miiDir,fileNameList[index]);
			}
		}

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}