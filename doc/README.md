
#### 0x00 Introduce

#### 0x01 page hardware
&emsp;risc-v 指令维护虚拟地址(用户态和内核态)。机器内存或者物理内存通过物理地址来寻址。risc-v page table hardware 通过将虚拟地址映射到物理地址来关联这两个地址。

&emsp;xv6 运行在Sv39 RISC-V上，意味着64位的虚拟地址只有39位是被使用的。最上方的25位没有被使用。在Sv39配置里面，一个RISC-V page table是一个容纳2^27个`page table entries(PET)`的array。每个PTE包含一个44位的物理页面编号(*physical page number/(PPN))和一些flags, page hardware 使用虚拟地址39位里面的前27位来在pagetable里面找到一个PTE，然后生成一个56位的物理地址（44位来自PPN，12位来自原始的虚拟地址）.page table的控制粒度是以4096个字节对其的一个chunk. 这样一个chunk被成为一个page。

&emsp;在Sv39 RISC-V 里面，虚拟地址的头25位是没有被用来做地址翻译的。物理地址同样有这样的空间用来做以后增长的: 这个root在PTE格式里面是用另外10个bit来控制的。RISC-V设计者用了这样一个数字，因为2^39bytes是512GB，这个对于一个应用程序来说完全足够了。对于硬件而言，2^56次方也满足大部分io或者RAM设备了。如果有需要，RISC-V 设计者也可以定义Sv48出来。

&emsp;正如3.2所示范。一个RISC-V cpu 将一个虚拟地址翻译成为一个物理地址主要经过下面三个步骤。一个page table在物理内存里面主要以三层树结构。树的root节点是一个4096字节大小的pagetable page，里面包含了512个PTE，这个里面存放是物理地址(用来在第二级的 tree里面寻找找pagetable)。第二级节点里面同样也是存放了513个PTE，这些PTE用来在第三级的节点里面寻找。page hardware 27个bit的前9个bit用来表识需要在第一层树里面选择多少个PTE， 中间的9个bit 用来表识在第二级的树里面选取多少个PTE，最后的9个bit 用来表识在第三级的树里面选择多少个PTEs(总共加起来27个bitp)


&emsp;如果这三个PTEs有任何一个不存在，那么pagehardware都会抛出一个page-fault 的异常出来，然后让内核来处理这个异常。(看第四章)

&emsp;三层结构比一层结构更为高效，在一个有着很大范围的虚拟地址空间里面，如果没有映射，三级结构可以忽略整个page directory(第一级别没找到，第二/三级别就不用去执行了)。例如，如果一个apps 用了很少的地址空间，并且只从0开始，那么从1-511整个page directory都是非法的，那么kernel 并不会为其余的511个page directory去分配内存，因此kernel也不会为剩下的2/3级目录里面去分配内存。因此在这个例子里面，三级设计至少可以节省大概511个页面，和剩余级里面的511\*512的页面目录。


&emsp;cpu在遍历三层结构在硬件层面作为执行加载/存储指令的一部分。三层结构一个潜在的缺点就是CPU必须需要在内存里面加载三个PTEs来执行地址翻译工作。为了避免加载的开销，cpu将这些PTEs缓存起来(TLB ).


&emsp;每个PTE 都包含了一些flags bit来告诉page hardware应该如何来访问虚拟地址(权限控制)。这些flags被定义在`kernel/risvc.h`里面

&emsp;为了告诉硬件使用一个page table， kernel 必须要把page -table的root节点的物理地址写入到`stap`寄存器里面.每个cpu都有自己的`vtap`寄存器。然后cpu将会转换后续所有生成的地址(之前的地址是物理地址，vtap设置之后，所有的地址就是虚拟地址).每个CPU都有自己的`vtap`寄存器，因此不同的cpu可以运行不同的process，每个不同的进程都有一个自己的私有地址，这个私有地址是由它自己的page table来描述的。

&emsp;典型的kernel将所有的内存空间映射到它自己的pagetable，因此它可以读取/写入内存任何一个地址通过加载/restore指令。由于page directories就在物理内存里面，因此kernel可以针对PTE内容进程编程，通过使用标准的store指令通过虚拟地址来写入内容。

&emsp;关于术语有几个比较重要的点: 物理内存是指DRAM里面的存储单元，物理内存里面的每个bytes都有一个地址，被称为物理地址。指令只采用虚拟地址，虚拟地址有paging hardware 来翻译成物理地址，然后将物理地址发送到DRAM硬件来读取/存储。和物理内存和虚拟地主不一样的是，虚拟内存不是严格意义伤的物理设备，而是指代内核里面提供对物理内存和虚拟地址管理的一种机制和抽象的集合。



#### kernal address
&emsp;xv6 为每个process都维护了一个page table，用来描述每个process 的用户态地址空间+一个用来描述内核地址空间的单页表。kernel配置它自己的地址空间，以便他可以访问物理内存以及各种硬件的虚拟地址( 硬件的虚拟地址是由厂商决定的，硬件的虚拟地址和物理地址是不变的映射 )。

&emsp;QEMU 模拟了一台计算机，包括了RAM，它的物理地址起始位置是从`0x80000000`，然后一直持续到`0x86400000`这段内存空间被成为`PHYSTOP`. QEMU 同样模拟了各种IO设备。QEMU通过内存映射控制注册器的方式将这些io设备接口暴露给软件(这些地址位于0x80000000下面。)  ， kernel  可以通过读写这些特殊的地址来和这些设备进行交互。第四章将会解释如何这些设备交互。

&emsp;kernel 使用`direct mapping`来获取memory-mapped设备的。directory mapping简化了内核代码关于读写物理内存这块的操作。例如`fork`，当`fork`为子进程分配用户态内存的时候，allocator直接返回那段内存的物理地址，fork直接将那段物理地址当作虚拟地址来使用，当拷贝parent的内存给子进程的时候。

有几种内核虚拟地址是不能映射的。
- `trampoline page`。它是被映射在虚拟地址的最顶部的。用户page table都有相同都这种映射，第四章将会讨论这种页面的机制，但是在这个地方我们将会看到它一个比较有意思的case。一个物理页面(包含了trampoline代码的)在内核人的地址空间被映射了两次一次在虚拟内存的最顶部，还有一次就是直接映射。
- kernel stack page。 每个process都有一个属于它自己的kernel stack,这些地址被映射的很好，以便在它的下面xv6可以留下一个没有被映射的保护页面。Guard的PTE是非法的，因为这个页面的`PTE_V`没有被设置，因此如果kernel 益处了kernel stack，它很有可能会抛出一个异常，然后kernel pannic。如果没有这个guard页面，一但栈溢出覆盖了kernel的内存空间，那么将会导致非常严重的问题。

&emsp;由于kernel通过高位内存映射来使用他的栈，他们也可以通过直接映射来使用这些内存。另外一种设计可能就是只有直接映射，然后在直接映射的地方来使用堆栈地址。在这种安排下，提供guard page将会引入为映射的虚拟地址，这些将会指向真实的物理地址，这个很难用。

&emsp;内核使用`P_TE_R`和`PTE_X`用来映射用做trampoline 和kernel text的pages。kernel 从这些pages里面读取和执行指令。kernel使用`PTE_R`和`PTE_W`来控制其他page的权限。针对guard的映射是无效的。


#### 创建一个地址空间
&emsp;xv6用来维护地址空间和page tables的代码位于vm.c里面。核心的数据结构`pagetable_t`这个指向root pagetable, 一个pagetable_t它可以是kernel的page table，也可以是每个process的pagetable. 核心函数位于`walk`，这个函数为虚拟地址找到一个`PTE`，和`mappages`函数，这个函数为一个新的mapping来安装一些PTEs.
