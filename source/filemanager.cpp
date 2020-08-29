#include "filemanager.hpp"

bool FileManager::OpenPath(string s){
    struct dirent *d;
    DIR *pdir = opendir(s.c_str());
    char *tmpName;
    entry tmpEntry;
    if(!pdir)return false;
    curPath = s;
    entries.clear();
    entries.shrink_to_fit();
    while(1){
        d = readdir(pdir);
        if(!d)break;
        tmpName = d->d_name;
        if(!(strcmp(tmpName,".") && strcmp(tmpName,"..")))continue;
        tmpEntry.name = string(tmpName);
        if(d->d_type == DT_DIR){
            tmpEntry.isDir = true;
        }else{
            tmpEntry.isDir = false;
        }
        entries.push_back(tmpEntry);
    }
    closedir(pdir);
    return true;
}

bool FileManager::Open(int i){
    return OpenPath(getFullPath(i));
}

FileManager::FileManager(string s){
    if(OpenPath(s)){
        valid = true;
    }else{
        valid = false;
    }
}

bool FileManager::Reload(){
    return OpenPath(curPath);
}

bool FileManager::Back(){
    int rootIndex = 0;
    int index = curPath.size() - 1;
    if(curPath[index] == '/')index--;
    while(curPath[index] != '/'){
        if(index < 1)return false;
        index--;
    }
    while(curPath[rootIndex] != '/'){
        rootIndex++;
    }
    if(rootIndex == index){
        return OpenPath(curPath.erase(index + 1));
    }
    return OpenPath(curPath.erase(index));
}

string FileManager::getFullPath(int i){
    if(curPath[curPath.size() - 1] == '/'){
        return curPath + (entries[i]).name;
    }
    return curPath + "/" + (entries[i]).name;
}