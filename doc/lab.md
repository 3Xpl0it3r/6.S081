


## Alarm(Hard)
&emsp;在这个练习里面,需要给xv6增加一个特性:周期性的提示一个信息,如果一个进程在使用CPU的话.这个对于限制计算的进程是非常有用的,例如也许想要限制进程使用有限的CPU(例如k8s里面CPUlimit),亦或者想要固定周期执行一些操作的进程(而不是一直占用).更普遍一点,你将会实现一个用户态的中断/故障处理的雏形.你可以使用类似的一些东西来处理应用程序里面的页面错误.`alarmtest`和`usertest`这两个测试用来检查解答对不对.


&emsp;你需要添加一个新的syscall, `sigalarm`(interval, handler).如果一个应用程序调用了`sigalarm(n, fn)`,然后应用程序在消耗n个CPU`trick`之后,`fn`函数就会被调用.当fn函数完成返回之后,应用程序又回从它离开的地方恢复过来.tick在xv6里面是任意的时间单位,这个取决于硬件产生时钟的频率.如果一个应用程序执行了`sigalarm(0,0)`,那么kernel会停止执行alarm.

&emsp;你会发现在xv6仓库里面有一个文件`kernel/alarmtest.c`.把它添加到Makefile里面.只有在你添加了`sigalarm`和`sigreturn`这两个syscall之后,`alarmtest`才能够被编译通过.(因此需要实现两个syscall, 一个是sigalarm,一个是sigreturn)

&emsp;`alarmtest`在`test0`里面会调用`sigalarm(2, periodic)`来要求kernel每个2个ticks就执行一次`periodic`函数,然后再自旋一会儿.你可以在`user/alarmtest.asm`里面找到对应的汇编代码,这个在debug的时候会用到.如果你执行`alarmtest`产生如下的输出,那么就意味着答案是正确的.

```bash
$ alarmtest
test0 start
........alarm!
test0 passed
test1 start
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
test1 passed
test2 start
................alarm!
test2 passed
$ usertests
...
ALL TESTS PASSED
$
```

&emsp;当你完成的时候你会发现你的代码只有短短几行,但是这个过程是非常棘手的.

## test0: Invoke handler
&esmp;通过修改内核来跳转到用户态的alarm handler代码处,这个会让test0打印一个`alarm!`.不用担心在输出`alarm!`之后会发生什么;(如果在输出完alarm之后Panic掉了也没关系,这个只是test0).一些提示如下:

- 需要在Makefile里面把`alarmtest`添加进去,和之前实验类似
- 在用户态添加正确的函数声明: `int sigalarm(int ticks, void(*handler))`和`int sigreturn(void)`;
- 更新`user/usys.pl`(用来生成user/usys.S),kernel/syscall.h以及`kernel/syscall.c`来允许`alarmtest`来调用sigalarm和sigreturn系统调用;
- For now, 你的`sigreturn`应该只是返回0.
- 你的`sys_sigalarm()`应该把alarm internal和handler的指针存放在proc新的字段里面;(kernel/proc.h)
- 你需要追踪自从上次调用alarm handler到现在,以及经历了多少个tick了,你也需要把这个加到`proc`里面,用一个新字段来保存它.  你可以在`allocproc`里面初始化`proc`字段(proc.c).
- 每次硬件时钟tick一次,它都会触发一次中断,这个中断处理函数在`usertrap()`里面(kernel/trap.c)
- 你只需要在有timer中断的情况下处理process的alarm ticks. `if (which_dev ==2) {doSth;}`
- 一个process只有在当前timer内未未完成的时候才会调用alarm函数.需要注意的是,用户告警函数可能是0(例如在user/alarmtest.asm里面periodic函数就是0)
- 你需要修改`usertrap()`函数,以便当到达timer ticker period的时候,用户进程可以触发handler函数. 当一个trap返回到用户态的时候,什么来决定用户空间代码恢复的地址(恢复用用户态,用户态应该从哪个地方开始继续执行,这个是由谁来决定).
- 如果在运用qemu指定CPU=1,那么在用gdb debug的时候会更容易查看`make CPUS=1 qemu-gdb`
- 如果alarmtest打印了`alarm!`,那么这个test就过了


## test1/test2
&emsp;在打印完`alarm!`之后test0/test1可能会panic掉,或者alarmtest最终打印了`test1 faild`亦或者test退出但是没有打印`test1 passed`. 为了解决这个问题,我们需要保证在alarm handler完成的时候,控制返回地址返回到用户程序最初在被timer中断的地方.我们必须要确保寄存器里面的值要被正确的恢复(恢复到被中断那刻的状态),以便用户程序在alarm之后可以正常继续执行. 最后我们应该重制alarm计数器,以便handler可以周期的执行.

&emsp;作为开头,我们已经替你做了一些设计决策:就是当用户程序在处理完handler的时候应该调用sigreturn 来返回;可以看看alarmtest.c里面periodc作为例子.这意味着你可以在`userteap`和`sys_sigreturn`里面添加代码,以便用户进程在处理完handler之后可以正常的恢复;

&emsp;一些提示:
- 代码里面需要保存和恢复寄存器(32个寄存器?)
- `usertrap`里面应该保存足够多的状态,以便在定时器结束的时候使用sigreturn可以正确的返回.
- 防止timer ticker handler被重复调用. 如果handler还没有返回,那么kernel就不应该再次调用它.(在test2里面会测试到这一点)

