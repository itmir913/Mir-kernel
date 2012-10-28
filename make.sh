echo ""
echo "#   #  #####  ######  ######  #####  #"
echo "# #    #      #    #  #    #  #      #"
echo "#      #####  ######  #    #  #####  #"
echo "# #    #      #    #  #    #  #      #"
echo "#   #  #####  #    #  #    #  #####  ###### # # #"
echo ""
#!/bin/bash
#######################################################
#  PANTECH defined Environment                        #
#######################################################
# export TARGET_BUILD_SKY_MODEL_NAME=IM-A750K
# export TARGET_BUILD_SKY_MODEL_ID=MODEL_EF32K
# export TARGET_BUILD_SKY_BOARD_VER=EF32K_ES20
# export TARGET_BUILD_SKY_FIRMWARE_VER=S0832143
# export TARGET_BUILD_SKY_CUST_INCLUDE=$PWD/include/CUST_SKY.h
# export TARGET_BUILD_SKY_CUST_INCLUDE_DIR=$PWD/include
#######################################################

ODIR=../build_ef30
image_filename="$ODIR/arch/arm/boot/zImage"
TOOLCHAIN=$HOME/arm-eabi-4.4.0/bin/arm-eabi-

export SKY_ANDROID_FLAGS="-DFEATURE_AARM_RELEASE_MODE -DMODEL_SKY -DMODEL_ID=MODEL_EF32K -DBOARD_VER=EF32K_ES20 -I$PWD/include -include $PWD/include/CUST_SKY.h -DFIRM_VER=\\\"S0832143\\\" -DSYS_MODEL_NAME=\\\"EF32K\\\" -DSKY_MODEL_NAME=\\\"IM-A750K\\\""
KERNEL_DEFCONFIG=msm7627-EF32K-perf_defconfig

if [ ! -d $ODIR ]; then
	mkdir $ODIR
	chmod 755 $ODIR
fi

if [ ! -f $ODIR/.config ]; then
	echo "-------------------------------"
	echo "Loading defconfig..."
	echo "-------------------------------"
	make O=$ODIR ARCH=arm CROSS_COMPILE=$TOOLCHAIN $KERNEL_DEFCONFIG 
fi

if [ "$1" = "" ]; then
	make -j4 O=$ODIR ARCH=arm CROSS_COMPILE=$TOOLCHAIN zImage
else
	make -j4 O=$ODIR ARCH=arm CROSS_COMPILE=$TOOLCHAIN $1 $2 $3
fi

if [ -f $image_filename ]; then
	echo "   making boot.img"
	./mkbootimg --cmdline "console=ttyHSL1,115200n8 androidboot.hardware=qcom loglevel=0 log_" --base 0x00200000 --pagesize 4096 --kernel $image_filename --ramdisk ramdisk.gz -o ../boot-new.img
	cp -f $ODIR/drivers/net/wireless/bcm4329/wlan.ko ..
fi

