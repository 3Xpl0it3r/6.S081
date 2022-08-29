image="l0calh0st.cn/xv6-riscv"
img_exist=`docker images|grep ${image}`
if [ -z "${img_exist}" ]
then
    echo "image ${image} is not existed, then build it ......"
    docker build . -t ${image} -f build/Dockerfile
fi

docker run -it --name xv6 -v `pwd`:/xv6/ ${image} bash
