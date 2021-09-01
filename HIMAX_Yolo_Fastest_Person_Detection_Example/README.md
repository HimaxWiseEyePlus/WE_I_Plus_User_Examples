# HIMAX Yolo-Fastest Person Detection Example
This example shows how to deploy your own yolo-fastest tflite model to HIMAX WE-I EVB.

- Prerequisites
  - This example will supports GNU and Metaware Development Toolkit. Please check the Development Toolkit chapter [here](https://github.com/HimaxWiseEyePlus/himax_tflm#prerequisites) to prepare the environment to build the example.
- Build the example and flash image.
  - Download related third party data (only need to download once)

    ```
    make download
    ```

  - Default building toolchain in makefile is Metaware Development toolkit, if you are trying to build example with GNU toolkit. please change the `ARC_TOOLCHAIN` define in `Makefile` like this

    ```
    #ARC_TOOLCHAIN ?= mwdt
    ARC_TOOLCHAIN ?= gnu
    ```
  
  - Train your own yolo-fastest model
    - Model used by this example is training with COCO dataset, please take a look [here](https://github.com/HimaxWiseEyePlus/Yolo-Fastest) about training detail.

  - Please put your own model and rename it to `yolo.tflite`, you can also download from [here](https://github.com/HimaxWiseEyePlus/Yolo-Fastest/blob/master/ModelZoo/yolo-fastest-1.1_160_person/yolo-fastest-1.1_160_person.tflite). Your folder structure should look like this:
    ```
    WE_I_Plus_User_Examples/HIMAX_Yolo_Fastest_Person_Detection_Example/
    |_ images
    |_ Makefile
    |_ README.md
    |_ yolo_fastest
    |_ yolo.tflite
    ```
  - Example with reading model from flash of HIMAX SDK, detail device initialization will be done by `hx_drv_flash_init()` and `hx_drv_flash_get_Model_address()` .
    - You should initial flash first. Then you can get the right model address.  
    - You can simply call them to initial flash and get model addess to retrieve the model data from flash.
  - Build this example, just key-in following command on the console. Flash image name will be `yolo*.img`.
    ```bash
    make yolo
    make flash example=yolo
    ```
  - Then, you should update the application, `yolo*.img` , in the flash.
    - Flash Image Update at Linux Environment [here](https://github.com/HimaxWiseEyePlus/bsp_tflu/tree/master/HIMAX_WE1_EVB_user_guide#flash-image-update-at-linux-environment)
- After above steps, update `yolo*.img` to HIMAX WE1 EVB. After get data from sensor, we can display them on the console and see the images with bounding box on the PC TOOL.
  - You can see your detect results on the console .
    - Console (minicom)

      ![alt text](images/minicom.png)


  - You can see your detect results with bounding box on the PC TOOL.
    - Before you click `PC_TOOL`, you can download [here](https://github.com/HimaxWiseEyePlus/WE_I_Plus_User_Examples/releases/download/v1.0/PC_TOOL), please key-in following command on console.
      ```bash
      chmod 777 PC_TOOL
      ```

    - Click `PC_TOOL` under your folder.

       ![alt text](images/pc_tool.png)
    - Please click `Recv` button.
      ![alt text](images/linux_pc_tool.png)

    - Then you will see your detect results with bounding box.
     ![alt text](images/linux_result.png)


