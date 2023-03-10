# HIMAX Yolo-Fastest Person Detection Example For Seeed Grove Vision AI Module
This example shows how to deploy your own yolo-fastest tflite model to **Seeed Grove Vision AI Module**.

## How to build the firmware?

This explains how you can build the firmware for Grove - Vision AI Module.

**Note:** The following has been tested to work on Ubuntu 20.04 PC

- **Step 1:** Install the following prerequisites

```sh
sudo apt install make
sudo apt install python3-numpy
```

- **Step 2:** Download GNU Development Toolkit

```sh
cd ~
wget https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases/download/arc-2020.09-release/arc_gnu_2020.09_prebuilt_elf32_le_linux_install.tar.gz
```

- **Step 3:** Extract the file

```sh
tar -xvf arc_gnu_2020.09_prebuilt_elf32_le_linux_install.tar.gz
```

- **Step 4:** Add **arc_gnu_2020.09_prebuilt_elf32_le_linux_install/bin** to **PATH**

```sh
export PATH="$HOME/arc_gnu_2020.09_prebuilt_elf32_le_linux_install/bin:$PATH"
```

- **Step 5:** Clone the following repository and go into HIMAX_Yolo_Fastest_Person_Detection_Example_For_Grove_AI folder

```sh
git clone https://github.com/HimaxWiseEyePlus/WE_I_Plus_User_Examples
cd WE_I_Plus_User_Examples/HIMAX_Yolo_Fastest_Person_Detection_Example_For_Grove_AI
```

- **Step 6:** Download related third party, tflite model and library data (only need to download once)

```sh
make download
```

- **Step 7:** Compile the firmware

```sh
make
make flash
```

This will generate **output.img** inside **tools/image_gen_cstm/output/** directory


- **Step 8:** Generate firmware image **grove_ai_yolo.uf2** file

```sh
python3 tools/ufconv/uf2conv.py -t 0 -c tools/image_gen_cstm/output/output.img -o grove_ai_yolo.uf2
```

- **Step 9:** Convert pre-trained model yolo_himax.tflite to **model-1.uf2** file

```sh
python3 tools/ufconv/uf2conv.py -f GROVEAI -t 1 -c yolo_himax.tflite -o model-1.uf2
```
## How to flash the firmware?

This explains how you can flash the firmware to Grove - Vision AI Module.

- **Step 1:** Connect Grove - Vision AI Module to the host PC via USB Type-C cable 

<div align=center><img width=460 src="https://files.seeedstudio.com/wiki/SenseCAP-A1101/47.png"/></div>

- **Step 2:** Double-click the boot button on Grove - Vision AI Module to enter mass storage mode

<div align=center><img width=220 src="https://files.seeedstudio.com/wiki/SenseCAP-A1101/48.png"/></div>

- **Step 3:** After this you will see a new storage drive shown on your file explorer as **GROVEAI**

<div align=center><img width=250 src="https://files.seeedstudio.com/wiki/SenseCAP-A1101/19.jpg"/></div>

- **Step 4:** Drag and drop the prevous **grove_ai_yolo.uf2** and **model-1.uf2** file to GROVEAI drive

Once the copying is finished **GROVEAI** drive will disapper. This is how we can check whether the copying is successful or not.

## How to view the camera stream? (Grove AI Family firmware)

- **Step 1:** After loading the firmware and connecting to PC, visit [this URL](https://files.seeedstudio.com/grove_ai_vision/index.html)

<div align=center><img width=1000 src="https://files.seeedstudio.com/wiki/SenseCAP-A1101/31.png"/></div>

- **Step 2:** Click **Connect** button. Then you will see a pop up on the browser. Select **Grove AI - Paired** and click **Connect**

<div align=center><img width=1000 src="https://files.seeedstudio.com/wiki/SenseCAP-A1101/32.png"/></div>

After that, you can see the real-time video stream on the web UI.

# Train your own yolo-fastest model
   - Model default used by this example is training with our own HIMAX dataset, please take a look [here](https://github.com/HimaxWiseEyePlus/Yolo-Fastest#himax-pretrained-model) about training detail.

  - The pre-trained model, `yolo_himax.tflite`, which is trained by our own HIMAX dataset, and you can also download from [here](https://github.com/HimaxWiseEyePlus/Yolo-Fastest/raw/master/ModelZoo/yolo-fastest-1.1_160_person/yolo-fastest-1_1_160_person_himax.tflite) and rename it to `yolo_himax.tflite`.