#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"
#include "../kernel/fs.h"
#include "../kernel/fcntl.h"
#include "../kernel/param.h"



#define MAXLENGTH 32        // 每个命令最大长度

int main(int argc, char* argv[])
{
    if (argc > MAXARG) {
        fprintf(2, "ERROR: argument too many > %d\n", MAXARG);
        exit(1);
    }

    // copy all params from argv
    char params[MAXARG][MAXLENGTH];
    memset(params, 0, MAXARG*MAXLENGTH);

    // command 
    char cmd[MAXLENGTH] = {0};
    memset(cmd, 0, MAXLENGTH* sizeof(char));

    int num_batch = -1;         // 批次
    int num_arg_cli = 0;            // xargs 命令里面参数的个数
    int num_stdin_arg = 0;      // 来自stdin里面参数个数

    int flag_cmd_has_set = -1;
    for(int i = 1; i < argc ; ++i){
        if (strcmp(argv[i], "-n") == 0){
            ++i;
            num_batch = atoi(argv[i]);
        }else if (flag_cmd_has_set == -1){
            strcpy(cmd, argv[i]);
            flag_cmd_has_set = 0;
        }else {
            strcpy(params[num_arg_cli], argv[i]);
            num_arg_cli++;
        }
    }


    // 参数来自两个地方， 1. xargs 后面的自带参数 2. stdin
    // 需要先执行前面一个, 读取参数拼接到后面
    char bufchar;                   // 逐个读取char buffer
    int bufindex = 0;               // index of read
    int flags_is_newline = -1;          // flag 是否是换行
    int  flag_split = -1;
    
    

    while (read(0, &bufchar, sizeof(char)) > 0) {
        if (flags_is_newline == 0 && bufchar == 'n') {
            flags_is_newline = -1;
            bufindex = 0;
            num_stdin_arg ++;
            continue;
        }
        if (bufchar == '\\')  {
            flags_is_newline = 0;
            continue;
        }
        if (bufchar == '\n'|| bufchar == ' ') {
            flag_split = 0;
            continue;
        }
        if (bufchar == '"') continue;
        if (flag_split == 0) {
            flag_split = -1;
            bufindex = 0;
            num_arg_cli++;
        }
        params[num_arg_cli+num_stdin_arg][bufindex++] = bufchar;
    }
    num_stdin_arg++;

    char *batch_args[MAXARG];
    batch_args[0] = cmd;
    for (int i = 0 ; i < num_arg_cli; i ++) {
        batch_args[1+i] = params[i];
    }
    int index = 0;

    if (num_batch> 0)
    {
        // 拼接批次执行的命令
        for (int i = 0 ; i < (int)(num_stdin_arg/num_batch); i++) {
            index = 0;
            for (int j = 0 ; j < num_batch; j++) {
                if (strlen(params[num_batch+i * num_batch + j]) == 0 ){
                    continue;
                }
                batch_args[1+num_arg_cli+index] = params[num_batch+i*num_batch + j];
                index ++;
            }
            batch_args[1+ num_arg_cli + index] = 0;
            int pid = fork();
            if (pid == 0) {
                exec(cmd, batch_args);
            }else {
                wait((int*)0);
            }
        }
    } 
    else {
        index = 0;
        for (int i =0 ; i < num_stdin_arg; i++) {
            if (strlen(params[num_arg_cli]) == 0) {
                continue;
            }
            batch_args[1 + num_arg_cli + index] = params[num_arg_cli + i];
            index ++;
        }

        batch_args[num_arg_cli+index + 1] = 0;
        if (fork() == 0) {
            exec(cmd, batch_args);
            exit(0);
        } else {
            wait((int*)0);
        }
    }
    exit(0);
}



