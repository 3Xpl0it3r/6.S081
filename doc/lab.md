## System call tracing
> 在这个作业里面你将需要添加一个tracing的功能,这个在你后续的lab里面可以用来debug.你将会创建一个新的trace的systemcall,这个可以用来控制tracing.它应该接受一个参数,一个整数`mask`,用来指定你需要trace的syscall.例如你需要trace fork这个系统调用,程序将会调用`trace(1 << SYS_fork)`,SYS_fork的数字编号您可以从`kernel/syscall.h`里面找到.你需要修改xv6的内核代码,以便在每个系统调用即将返回的时候打印一行信息,如果您在mask被设置了SYStem call的数字. 这行信息应该包括processid和sysemcall的名称,以及返回值.你不需要打印系统调用的参数.Trace调用应该trace调用trace的进程及其这个进程后面产生的任何子进程.但是它不应该影响其他进程.

Hints:
- Add $U/_trace to UPROGS in Makefile

- Run make qemu and you will see that the compiler cannot compile user/trace.c, because the user-space stubs for the system call don't exist yet: add a prototype for the system call to user/user.h, a stub to user/usys.pl, and a syscall number to kernel/syscall.h. The Makefile invokes the perl script user/usys.pl, which produces user/usys.S, the actual system call stubs, which use the RISC-V ecall instruction to transition to the kernel. Once you fix the compilation issues, run trace 32 grep hello README; it will fail because you haven't implemented the system call in the kernel yet.

- Add a sys_trace() function in kernel/sysproc.c that implements the new system call by remembering its argument in a new variable in the proc structure (see kernel/proc.h). The functions to retrieve system call arguments from user space are in kernel/syscall.c, and you can see examples of their use in kernel/sysproc.c.

- Modify fork() (see kernel/proc.c) to copy the trace mask from the parent to the child process.

- Modify the syscall() function in kernel/syscall.c to print the trace output. You will need to add an array of syscall names to index into.


## Sysinfo
> 在这个作业里面你需要添加一个syscall,`sysinfo`,这个用来收集当前正在运行系统的信息. 这个syscall只接收一个参数:一个指向`struct sysinfo`的指针(kernel/sysinfo.h).kernel应该填充这个结构体里面所有的字段:`freemem`代表当前整个系统free内存的总数,`nproc`这个字段用来统计所有处于`UNUSED` 状态的process.我们提供了一个`sysinfotest`测试代码.如果它打印了`sysinfotest: OK`,那么这个作业您就完成了

Hints:

- Add $U/_sysinfotest to UPROGS in Makefile(添加`$U/_sysinfotest` 到Makefile 文件里面的UPROGS 下面)

- Run make qemu; user/sysinfotest.c will fail to compile. Add the system call sysinfo, following the same steps as in the previous assignment. To declare the prototype for sysinfo() in user/user.h you need predeclare the existence of struct sysinfo:

```c
struct sysinfo;
int sysinfo(struct sysinfo *);
```
  
- Once you fix the compilation issues, run sysinfotest; it will fail because you haven't implemented the system call in the kernel yet.
sysinfo needs to copy a struct sysinfo back to user space; see sys_fstat() (kernel/sysfile.c) and filestat() (kernel/file.c) for examples of how to do that using copyout().

- To collect the amount of free memory, add a function to kernel/kalloc.c

- To collect the number of processes, add a function to kernel/proc.c
