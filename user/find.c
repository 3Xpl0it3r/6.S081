#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


void find(char* path , char* target);




int main(int argc, char* argv[])
{

  if(argc < 3){
    fprintf(2, "ERROR: arugment is not enough");
    exit(0);
  }
  // usage find path filename
  char* path = argv[1];
  char* target = argv[2];
  find(path, target);
  exit(0);

}

// copy from ls.c
char *fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;
    // Find first character after slash
    for (p=path + strlen(path); p >= path && *p != '/' ; p--) ;
    p++;

    if(strlen(p)>=DIRSIZ) 
        return p;
    memmove(buf, p ,strlen(p));
    // replace ' ' with '\0'
    memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
    return buf;
}


void find(char* path, char* target){
    struct stat st;
    struct dirent de; 
    
    char buf[512], *p;

    int fd = 0;
    // open path
    if((fd = open(path, O_RDONLY))< 0) {
        fprintf(2, "ERROR: open path %s failed\n", path);
        return;
    }
    // get path state
    if (fstat(fd, &st) < 0) {
        fprintf(2, "ERROR: get fstat failed");
        close(fd);
        return;
    }
     
    switch (st.type) {
        case T_FILE:
            if (strcmp(fmtname(path), target) == 0) {
                fprintf(1, "%s\n", path, st.type, st.ino, st.size);
            }
            break;
        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 >  sizeof(buf)){
                fprintf(2, "find: path is too long");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(path);
            *p ++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum ==0)
                    continue;
                memmove(p, de.name, DIRSIZ); // de.name
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0) {
                    fprintf(2, "find: cannot state: %s\n", buf);
                    continue;
                }
                if ((strcmp(de.name, ".")!=0) && strcmp(de.name, "..")!=0) {
                    find(buf, target);
                }
            }
            break;
    }

    close(fd);
}
