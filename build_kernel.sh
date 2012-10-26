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
# export SKY_ANDROID_FLAGS="-DMODEL_ID=$TARGET_BUILD_SKY_MODEL_ID -DBOARD_VER=$TARGET_BUILD_SKY_BOARD_VER -I$TARGET_BUILD_SKY_CUST_INCLUDE_DIR -include $TARGET_BUILD_SKY_CUST_INCLUDE"
# if [ "$TARGET_BUILD_VARIANT" == "user" ]
# then
# 	 export SKY_ANDROID_FLAGS="-DFEATURE_AARM_RELEASE_MODE $SKY_ANDROID_FLAGS -DSYS_MODEL_NAME=\\\"$TARGET_BUILD_SYS_MODEL_NAME\\\" -DSKY_MODEL_NAME=\\\"$TARGET_BUILD_SKY_MODEL_NAME\\\""
# fi
#######################################################

# export SKY_ANDROID_FLAGS
export SKY_ANDROID_FLAGS="-DFEATURE_AARM_RELEASE_MODE -DMODEL_SKY -DMODEL_ID=MODEL_EF32K -DBOARD_VER=EF32K_ES20 -I$PWD/include -include $PWD/include/CUST_SKY.h -DFIRM_VER=\\\"S0832143\\\" -DSYS_MODEL_NAME=\\\"EF32K\\\" -DSKY_MODEL_NAME=\\\"IM-A750K\\\""

# Define KERNEL Configuration (depending on the SKY MODEL)
KERNEL_DEFCONFIG=msm7627-EF32K-perf_defconfig

# Build LINUX KERNEL
mkdir -p ./obj/KERNEL_OBJ/
make O=./obj/KERNEL_OBJ ARCH=arm CROSS_COMPILE=arm-eabi- $KERNEL_DEFCONFIG
#make O=./obj/KERNEL_OBJ ARCH=arm CROSS_COMPILE=arm-eabi- menuconfig
#cp ./obj/KERNEL_OBJ/.config ./kernel/$KERNEL_DECONFIG
make -j4 O=./obj/KERNEL_OBJ ARCH=arm CROSS_COMPILE=arm-eabi-

# Copy compressed linux kernel (zImage)
cp -f ./obj/KERNEL_OBJ/arch/arm/boot/zImage .
