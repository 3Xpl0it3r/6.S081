# l0calh0st.cn/xv6-riscv
FROM ubuntu:20.04

ENV DEBAIN_FRONTEND="noniteractive"

RUN apt-get update -y && apt-get install -y tzdata && ln -fs /usr/share/zoneinfo/Asia/Shanghai /etc/localtime  &&     dpkg-reconfigure --frontend noninteractive tzdata && apt-get install vim git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu  -y

WORKDIR "/xv6"

CMD ["/bin/bash"]
