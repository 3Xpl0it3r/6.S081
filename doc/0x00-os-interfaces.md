**Char1 Operating system interfaces**

&emsp;os的职责通过提供一些特色的服务,可以运行在计算机上的各种服务共享同一个硬件资源.
&emsp;os通过一个接口向用户提供服务，因此设计一个良好的接口是非常复杂的:一方面希望这个接口是非常简单和有限的，另外一方面又希望向用户提供更多的特性。解决问题的方案：依赖少数机制的接口，通过组合这些接口来提供更多的功能。



#### 0x00 Process and memory
&emsp;一个xv6程序包含了用户态内存(指令,数据,stack)和每个进程在内核态都有的私有的状态信息.`XV6`是一个分时共享程序，它透明的在一组等待执行的进程之间切换可用的CPU,如果一个进程不在执行了,那么它会保存这个进程的CPU寄存器,当再次运行这个程序的时候 ,它又回恢复这些状态.kernel 会将一个进程唯一标识符关联到每个进程.

**fork**
&emsp;`fork`这个syscall用来创建一个进程.子进程和父进程拥有完全一摸一样的内存空间(相同的指令,相同的数据).在父进程里面返回子进程的pid,在子进程里面返回0.(尽管父子进程有着相同的内存内容,但是他们运行在不同的内存空间,有着不同的寄存器状态) 用法如下:
```c
int pid = fork();
if (pid > 0 ){
    printf("parent: child=%d\n", pid);
    pid = wait((int*)0);
}else if(pid == 0){
    printf("child exiting \n");
}else {
    printf("fork error");
}
```

**exit**
&emsp;`exit`syscall会暂停当前程序继续执行，然后释放所有的内存资源/文件资源描述符.

**wait**
&emsp;`int wait(int* status)`wait这个syscall返回退出的进程的pid,并且把它的退出的状态吗保存到status里面。如果用户不太关心进程退出的状态吗，可以直接传一个0进去.

**exec**
&emsp;`int exec(char *file, char*argv[])`exec系统调用会从文件系统上面加载一个文件加载到内存里面用来替换原来进程的内存空间(但是这个文件格式必须是特色的格式,Linux上是ELF格式)。

> fork 会申请和父进程同样大的内存空间,exec会申请能够容纳镜像文件大小的内存，在runtime阶段,内存不够用的时候可以调用`malloc`->`sbrk(n)`会申请n大小的内存,返回这片内存的地址

#### 0x01 I/O and File Descriptors
&emsp;文件描述符用一个整数来表示内核管理的一个对象,程序可以在这个对象上面进行读写操作.一个程序可以通过下面几种方法来获取一个文件描述符:打开一个文件/目录/设备,或者创建一个pipe或者复制一个已经存在的文件描述符.(在xv6里面每个进程表里面都有个文件描述符索引,从0开始,默认0->标准输入,1->标准输出,2->标准错误), 每个文件描述符都有一个相关联的偏移量.

**int read(fd,buf,n)**
&emsp;从文件描述符里面最多读取n个字节，然后拷贝到buf里面，然后返回读取了多少个字节.接下来的read会从当前偏移量继续读取，如果没有内容它会返回0.

**int write(fd,buf,n)**
&emsp;从buf里面读取n个字节，然后写入到fd里面，返回写了多少个字节.和read一样，每次write之后都指向先前写入后的偏移量.

**例子**
```c
char buf[512];
int n;
for(;;){
    n = read(0, buf, sizeof(buf));
    if (n == 0){ // 已经读取完成了
        break;
    }
    if (n < 0){ // n小于0 ，读取错误
        fprintf(2, "read error \n");
        exit(-1);
    }
    if (write(1, buf, n) != n){ // 通过write输出到标准输出
        //  读取错误
        fprintf(2, "write error\n");
        exit(-2);
    }
}
```
**int close(fd);**
&emsp;close用于释放一个文件描述符,以便它可以在将来的`open`,`pipe`,`dup`等syscall里面被重复使用.

**int dup(fd)**
&emsp;用于复制一个已经存在的文件描述符,然后返回一个新的文件描述符,新旧文件描述符指向的是同一个底层资源对象(这两个文件描述符共享同样的文件偏移量和fork里面行为一个，在fork里面父子进程里面的文件描述符指向的是同一个底层对象)


#### 0x02 Pipe
**int pipe(int p[])**
&emsp;`pipe`是一个微型内核buffer，通过一对文件描述符的形式暴露给用户程序使用.一个读，一个写。从一端开始写，从另外一端读取。
**例子**
```c 
int p[2];
char *argv[2];

argv[0] = "wc";
argv[1] = 0;

pipe(p);
if (fork() == 0) { // 子进程
    close(0);       // 关闭0/标准输入 文件描述符
    dup(p[0]);      // 此时0指向的就是p[0] 
    close(p[0]);    // 关闭p[0]
    close(p[1]);    // 关闭p[1]
    exec("/bin/wc", argv); // 从标准输入读取参数(此时标准输入就是p[0])
} else { // 父进程
    close(p[0]);    // 关闭读
    write(p[1], "hello world\n",12); // 写
    close(p[1]); // 关闭写
}
```
&emsp;如果没有数据，`read`一个pipe会一直等待到有数据，或者所有的指向写管道的文件描述符都被关闭了,如果是后面一种情况，`read()`会返回0.


#### 0x03  File System
&emsp;`XV6`系统提供了文件:里面是一堆无解释的字节数组,和目录:里面包含了一堆名称,这些名称指向文件和其他目录。在`xv6`里面目录是一个树形结构,从一个被成为根节点的地方开始。

**mkdir**
&emsp;`int mkdir(char *dir)`创建一个目录

**mknode**
&emsp;`int mknode(char *file, int,int)`创建一个设备.(一个设备有两个数字，一个主设备标示，一个副设备标示).特别的是当一个程序打开一个设备文件描述符的时候，`read`和`write`系统调用会将请求传递给内核设备相关的实现，而不是转给文件系统.


&emsp;文件的名称和文件本身是不一样的。相同的底层文件被成为一个`inode`,但是它可以有很多个文件名称 (被称为links). 每个link都包含一个目录的条目。每个条目包含一个文件的名称，和一个指向inode的引用。一个inode包含一个文件所有元数据的相关信息，包括它的类型(是目录，还是文件，还是设备),它的长度，在磁盘上的位置，有多少个links指向这个文件

**fstat**
&emsp;`fstat`syscall 可以获取一个inode所有的信息，定义如下:
```c 
#define T_DIR 1 // 目录
#define T_FILE 2 // 文件
#define T_DEVICE 3 //设备

struct stat {
    int dev;        // 
    uint ino;   // Inode number
    short type;         //文件类型
    short nlink;        //有多少个links 指向这个文件
    uint64 size;        // 文件的长度
}
```
**link**
&emsp;`link`系统调用会创建一个新的文件系统名称，但是它指向的是和已经存在的文件相同的inode.
```c
open("a", O_CREATE|O_WRONLY);link("a", "b");
```

**unlink**
&emsp;`unlink`将会移除一个文件系统名称，但是link个数为0 的，时候，那么这个inode信息以及在磁盘上存储inode信息的相关资源也会被释放(也就说磁盘上有一个地方是用来存放 inode信息的，但是link个数为0，这个地方就会被释放)，导致没有任何引用指向底层的文件了(程序退出的时候这个文件也会被释放掉)



