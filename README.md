# RaspberryPi-4B-drive

## 本地编译

### 编译环境安装
```sh
sudo apt install git bc bison flex libssl-dev make
```

### 代码下载
```sh
git clone --depth=1 https://github.com/raspberrypi/linux
```

### 编译
```sh
cd linux
KERNEL=kernel7l
make bcm2711_defconfig
make -j4 zImage modules dtbs
```

## 交叉编译
### 编译环境安装
```sh
sudo apt install git bc bison flex libssl-dev make libc6-dev libncurses5-dev
```
### 编译
```sh
cd linux
KERNEL=kernel7l
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2711_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-  zImage modules dtbs -j4
#交叉编译需要指定 ARCH 与 CROSS_COMPILE
```

## 模块编译（驱动编译）
```sh
cd linux
KERNEL=kernel7l
make bcm2711_defconfig
make modules_prepare
#如果只是编译驱动，无需编译编译整内核
```