	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0   # 将stack0地址加载到SP寄存器
        li a0, 1024*4  # a0 + 4096
	csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0  # sp = sp + 4096
        # 上面作用开辟栈空间,大小4096
	# jump to start() in start.c
        call start      # 调用start函数
spin:
        j spin
