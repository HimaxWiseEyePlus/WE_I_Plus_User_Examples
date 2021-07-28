# HIMAX WE1 EVB Hello world Example 

In the example, we experiment with the UART console display "Hello world!" among HIMAX WE1 EVB. The example has been tested in Ubuntu 20.04 LTS environment.
  
## Table of contents
  - [Prerequisites](#prerequisites)
  - [Deploy to Himax WE1 EVB](#deploy-to-himax-we1-evb)
   
## Prerequisites
- Make Tool version
  
  A `make` tool is required for deploying Tensorflow Lite Micro applications, See
[Check make tool version](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/tools/make/targets/arc/README.md#make-tool)
section for a proper environment.

- Development Toolkit
  
  Install one of the toolkits listed below:
  
  - MetaWare Development Toolkit

    See
[Install the Synopsys DesignWare ARC MetaWare Development Toolkit](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/tools/make/targets/arc/README.md#install-the-synopsys-designware-arc-metaware-development-toolkit)
section for instructions on toolchain installation.

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
- Serial Terminal Emulation Application

  There are 2 main purposes for HIMAX WE1 EVB Debug UART port, print application output and burn application to flash by using xmodem send application binary.

## Deploy to Himax WE1 EVB

The example project for HIMAX WE1 EVB platform can be generated with the following command:

Download related third party data and model setting (only need to download once)

```
make download
```

Default building toolchain in the makefile is GNU toolkit, please switch to `mwdt` if you want to build it with Metaware Development toolkit

Build hello world example and flash image, the flash image name will be `hello_world.img`

```
make hello_world
make flash example=hello_world
```

