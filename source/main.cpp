#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include "filemanager.hpp"
#include "mii.hpp"

#define SHOW_FILE_NUM 15
#define MAX_LINE_LENGTH 70
#define MII_DIR "sd:/Miis"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

bool sdInitialize = false;
bool isfsInitialize = false;

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

void printFileList(int index,FileManager*fm){
    int page = index / SHOW_FILE_NUM,i;
	int curIndex;
	char showString[MAX_LINE_LENGTH - 5 + 1];
	std::string curPath;
	for(i = 0;i < SHOW_FILE_NUM;i++){
		curIndex = i + page * SHOW_FILE_NUM;
        if(curIndex >= fm->entries.size())break;
		if(curIndex == index){
            printf("-->> ");
		}else{
			printf("     ");
		}
		if(fm->entries[curIndex].isDir){
			curPath = fm->entries[curIndex].name + "/";
		}else{
			curPath = fm->entries[curIndex].name;
		}
		memset(showString,0,MAX_LINE_LENGTH - 5 + 1);
		memcpy(showString,curPath.c_str(),MAX_LINE_LENGTH - 5 > curPath.size() ? curPath.size() : MAX_LINE_LENGTH - 5);
		printf("%s\n",showString);
	}
	return;
}

void updateFileNameList(int index,FileManager*fm){
	int i;
    printf("\x1b[8;0H");
	for(i = 0;i < SHOW_FILE_NUM;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[8;0H");
	printFileList(index,fm);
	printf("\x1b[%d;0H",SHOW_FILE_NUM + 9);
	for(i = 0;i < 2;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[%d;0H",SHOW_FILE_NUM + 9);
	printf("A / Install Mii | UP DOWN / Select Mii\n");
	printf("HOME / Exit\n");
}

void updateCurPath(FileManager*fm){
	char showString[MAX_LINE_LENGTH + 1];
	memset(showString,0,MAX_LINE_LENGTH + 1);
	printf("\x1b[6;0H");
	printf("                                                                      \n");
	printf("\x1b[6;0H");
	memcpy(showString,fm->curPath.c_str(),MAX_LINE_LENGTH > fm->curPath.size() ? fm->curPath.size() : MAX_LINE_LENGTH);
	printf("%s",showString);
}

bool appInit(){
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
	printf("| Mii Importer v1.0.2 |\n");
    printf("| developed by Kazuki |\n");
    printf("+---------------------+\n");
    printf("[*] Initializing ISFS subsystem...");
	if(ISFS_Initialize() == ISFS_OK){
        printf(" OK!\n");
		isfsInitialize = true;
	}else{
		printf(" Error!\n");
		return false;
	}
	printf("[*] Mounting SD Card...");
    if (SD_Initialize()){
		printf(" OK!\n");
		sdInitialize = true;
	}else{
		printf(" Error!\n");
		return false;
	}
	DIR*pdir = opendir(MII_DIR);
	if(pdir){
        closedir(pdir);
	}else{
		if(mkdir(MII_DIR,0777) != 0){
            printf("\nError:mkdir\n");
			return false;
		}
	}
	return true;
}

void appExit(){
    if(sdInitialize){
        SD_Deinitialize();
	}
	if(isfsInitialize){
        ISFS_Deinitialize();
	}
	exit(0);
}

bool checkExt(const char *s){
    const char *ptr = strlen(s) + s - 1;
	while(1){
		if(*ptr == '.' || ptr <= s)break;
		ptr--;
	}
	if(!strcmp(ptr,".mii")){
        return true;
	}else if(!strcmp(ptr,".MII")){
        return true;
	}else if(!strcmp(ptr,".miigx")){
        return true;
	}else if(!strcmp(ptr,".MIIGX")){
        return true;
	}else if(!strcmp(ptr,".mae")){
        return true;
	}else if(!strcmp(ptr,".MAE")){
        return true;
	}else if(!strcmp(ptr,".rkg")){
        return true;
	}else if(!strcmp(ptr,".RKG")){
        return true;
	}
	return false;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
    bool noError = appInit();
	FileManager fm(MII_DIR);
	if(fm.valid == false){
		noError = false;
	}
    int index = 0,i;
    if(noError){
    printf("\x1b[6;0H");
        for(i = 0;i < 12;i++){
            printf("                                        \n");
	    }
	    updateCurPath(&fm);
		printf("\x1b[7;0H");
	    printf("----------------------------------------------------------------------\n");
		printf("\x1b[%d;0H",SHOW_FILE_NUM + 8);
		printf("----------------------------------------------------------------------\n");
		printf("\x1b[8;0H");
		printFileList(index,&fm);
		printf("\x1b[%d;0H",SHOW_FILE_NUM + 9);
		printf("A / Install Mii | UP DOWN / Select Mii\n");
		printf("HOME / Exit\n");
        printf("\x1b[7;0H");
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
		if(noError){
            if ( pressed & WPAD_BUTTON_UP ){
                if(index > 0){
                    index--;
					updateFileNameList(index,&fm);
				}
			}else if(pressed & WPAD_BUTTON_DOWN){
                if(index < fm.entries.size() - 1){
                    index++;
					updateFileNameList(index,&fm);
				}
			}else if((pressed & WPAD_BUTTON_A) && (fm.entries.size() > 0)){
				if(fm.entries[index].isDir){
					if(fm.Open(index)){
						index = 0;
						updateFileNameList(index,&fm);
						updateCurPath(&fm);
					}
				}else{
					printf("\x1b[%d;0H",SHOW_FILE_NUM + 9);
					for(i = 0;i < 2;i++){
                        printf("                                        \n");
	                }
					printf("\x1b[%d;0H",SHOW_FILE_NUM + 9);
					if(checkExt(fm.entries[index].name.c_str())){
					    printf("Installing Mii...\n");
				        installMii(fm.getFullPath(index).c_str());
				    }else{
					    printf("This is not a Mii file\n");
				    }
				}
			}else if(pressed & WPAD_BUTTON_B){
				if(fm.Back()){
					index = 0;
					updateFileNameList(index,&fm);
					updateCurPath(&fm);
				}
			}
		}

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}