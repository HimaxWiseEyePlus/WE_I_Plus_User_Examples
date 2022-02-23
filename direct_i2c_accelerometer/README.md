# HIMAX TensorFlow Lite for Microcontrollers
It is a modified version of the [TensorFlow Lite for Microcontrollers](https://github.com/tensorflow/tflite-micro) for use with HIMAX WE-I Plus Boards. Each example in the package has been tested in Ubuntu 20.04 LTS environment.

Following examples are included :
- magic wand example with direct I2C control to Lsm9ds1
  
## Table of contents
  - [Prerequisites](#prerequisites)
  - [Deploy to Himax WE1 EVB](#deploy-to-himax-we1-evb)


## Prerequisites
- Make Tool version
  
  A `make` tool is required for deploying Tensorflow Lite Micro applications, See
[Check make tool version](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/tools/make/targets/arc#make-tool)
section for proper environment.

- Development Toolkit
  
  Install the toolkits listed below:

  - GNU Development Toolkit

    See
[ARC GNU Tool Chain](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain) section for more detail, current released GNU version is [GNU Toolchain for ARC Processors, 2020.09](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases/download/arc-2020.09-release/arc_gnu_2020.09_prebuilt_elf32_le_linux_install.tar.gz). After download and extract toolkit to local space, please remember to add it to environment PATH. For example:

    ```
    export PATH=[location of your ARC_GNU_ROOT]/bin:$PATH
    ```

- curl command
  
  Installing curl for Ubuntu Linux.
  ```
  sudo apt update
  sudo apt upgrade
  sudo apt install curl
  ```
- make command
  
  Installing make for Ubuntu Linux. (If make is not installed)
  ```
  sudo apt install make
  ```
- Serial Terminal Emulation Application

  There are 2 main purposes for HIMAX WE1 EVB Debug UART port, print application output and burn application to flash by using xmodem send application binary.

## Deploy to Himax WE1 EVB

The example project for HIMAX WE1 EVB platform can be generated with following command:

Download related third party data and model setting (only need to download once)

```
make download
```

Build magic wand example and flash image, flash image name will be `magic_wand.img`

```
make magic_wand
make flash example=magic_wand
```

by this example, we directly use I2C to control imu, please search `exmaple/magic_wand/imu_lsm9ds1.c` for more detail.

After flash image generated, please download the flash image file to HIMAX WE1 EVB by UART, details are described [here](https://github.com/HimaxWiseEyePlus/bsp_tflu/tree/master/HIMAX_WE1_EVB_user_guide#flash-image-update-at-linux-environment)

