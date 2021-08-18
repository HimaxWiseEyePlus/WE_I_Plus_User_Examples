#=============================================================
# Toolchain definitions
#=============================================================
#ARC_TOOLCHAIN ?= mwdt
ARC_TOOLCHAIN ?= gnu

ifeq ($(ARC_TOOLCHAIN), mwdt)
	CC = ccac
	AR = arac
	CXX = ccac
	LD = ccac
else ifeq ($(ARC_TOOLCHAIN), gnu)
	CC := arc-elf32-gcc
	AR := arc-elf32-ar
	CXX := arc-elf32-g++
	LD := arc-elf32-ld
endif


RM = rm -rf
DL = curl -Ls -f
UNZIP = unzip 
CP = cp -r

SDK_LINK = https://www.himax.com.tw/we-i/himax_we1_sdk_v18.zip
SDK_LOC = .
SDK_NAME = sdk.zip

TOOL_LINK = https://www.himax.com.tw/we-i/image_gen_linux_v2_1_10.zip
TOOL_LOC = .
TOOL_NAME = image_gen.zip

DEPEND_LINK = https://github.com/foss-for-synopsys-dwc-arc-processors/embarc_mli/releases/download/Release_1.1/mw_gnu_dependencies.zip
DEPEND_LOC =  third_party/
DEPEND_NAME = gnu_dependencies.zip

TF_LINK = https://github.com/HimaxWiseEyePlus/himax_tflm/archive/f4c05b8eee81431292bf1c7e5e7d4e721ad7c201.zip
TF_LOC = ./
TF_NAME = himax_tflm.zip

MODEL_LINK = https://www.himax.com.tw/we-i/himax_tflu_tree/model_setting_v02.zip
MODEL_LOC = tensorflow/lite/micro/tools/make/downloads/
MODEL_NAME = model.zip

LIB_LINK = https://www.himax.com.tw/we-i/himax_tflu_tree/thrid_party_lib_v01.zip
LIB_LOC = third_party/
LIB_NAME = lib.zip

GEN_TOOL_DIR = image_gen_linux
ifeq ($(ARC_TOOLCHAIN), mwdt)
GEN_TOOL_NAME = image_gen
else ifeq ($(ARC_TOOLCHAIN), gnu)
GEN_TOOL_NAME = image_gen_gnu
endif
#=============================================================
# Files and directories
#=============================================================

LORA_SRCS := \
main.cc \
main_functions.cc \
tensorflow/lite/micro/kernels/arc_mli/scratch_buffers.cc \
tensorflow/lite/micro/kernels/arc_mli/scratch_buf_mgr.cc \
tensorflow/lite/micro/kernels/arc_mli/mli_slicers.cc \

LORA_OBJS := \
$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(LORA_SRCS)))

#=============================================================
# Applications settings
#=============================================================

MLI_PATH = third_party/arc_mli_package/bin/himax_arcem9d_r16/release
DEPEND_PATH = third_party/mw_gnu_dependencies/gnu_depend_lib
SDK_PATH = himax_we1_sdk
LIB_PATH = lib

ifeq ($(ARC_TOOLCHAIN), mwdt)
CXXFLAGS+= \
-Wstrict-aliasing \
-DTF_LITE_STATIC_MEMORY \
-Wsign-compare \
-Wdouble-promotion \
-Wshadow \
-Wunused-function \
-DNDEBUG \
-O3 \
-fno-rtti \
-DSCRATCH_MEM_Z_SIZE=0x10000 \
-DNDEBUG \
-g \
-DCPU_ARC \
-Hnosdata \
-DTF_LITE_STATIC_MEMORY \
-tcf=$(SDK_PATH)/arcem9d_wei_r16.tcf \
-Hnocopyr \
-Hpurge \
-Hcl \
-fslp-vectorize-aggressive \
-ffunction-sections \
-fdata-sections \
-tcf_core_config \
-I. \
-I./$(SDK_PATH) \
-I./$(LIB_PATH) \
-I./third_party/gemmlowp \
-I./third_party/flatbuffers/include \
-I./third_party/ruy \
-I./third_party/arc_mli_package/include \
-I./third_party/arc_mli_package/include/api \
-I./tensorflow/lite/micro/tools/make/downloads/kissfft


CCFLAGS+= \
-DTF_LITE_STATIC_MEMORY \
-DNDEBUG \
-O3 \
-DNDEBUG \
-g \
-DCPU_ARC \
-Hnosdata \
-DTF_LITE_STATIC_MEMORY \
-tcf=$(SDK_PATH)/arcem9d_wei_r16.tcf \
-Hnocopyr \
-Hpurge \
-Hcl \
-fslp-vectorize-aggressive \
-ffunction-sections \
-fdata-sections \
-tcf_core_config \
-I. \
-I. $(SDK_PATH)\
-I. $(LIB_PATH) \
-I./third_party/gemmlowp \
-I./third_party/flatbuffers/include \
-I./third_party/ruy \
-I./third_party/arc_mli_package/include \
-I./third_party/arc_mli_package/include/api \
-I./tensorflow/lite/micro/tools/make/downloads/kissfft

LDFLAGS+= \
$(MLI_PATH)/libmli.a \
-Hheap=8192 \
-Hnocopyr \
-m \
-Hldopt=-Coutput=$(MAP_NAME).map \
memory.lcf \
-Hldopt=-Bgrouplib \
$(SDK_PATH)/libboard_open_socket.a \
$(SDK_PATH)/libcpuarc.a \
$(SDK_PATH)/libbss.a \
$(SDK_PATH)/libboard_socket.a \
$(SDK_PATH)/liblibcommon.a \
$(SDK_PATH)/liblibaudio.a \
$(SDK_PATH)/liblibsecurity.a \
$(SDK_PATH)/liblibsensordp.a \
$(SDK_PATH)/liblibtflm.a

else ifeq ($(ARC_TOOLCHAIN), gnu)
CXXFLAGS += \
-fno-rtti \
-fno-exceptions \
-fno-threadsafe-statics \
-fno-unwind-tables \
-ffunction-sections \
-fdata-sections \
-fmessage-length=0 \
-DTF_LITE_STATIC_MEMORY \
-DTF_LITE_DISABLE_X86_NEON \
-O3 \
-Wsign-compare \
-Wdouble-promotion \
-Wshadow \
-Wunused-variable \
-Wmissing-field-initializers \
-Wunused-function \
-Wswitch \
-Wvla \
-Wall \
-Wextra \
-Wstrict-aliasing \
-Wno-unused-parameter \
-DREDUCE_CODESIZE \
-mxy \
-include $(SDK_PATH)/core_config.h \
-mcpu=em4_fpus \
-mlittle-endian \
-mcode-density \
-mdiv-rem \
-mswap \
-mnorm \
-mmpy-option=6 \
-mbarrel-shifter \
-mfpu=fpus_all \
-I. \
-I./$(SDK_PATH) \
-I./$(LIB_PATH) \
-I./third_party/gemmlowp \
-I./third_party/flatbuffers/include \
-I./third_party/ruy \
-I./third_party/arc_mli_package/include \
-I./third_party/arc_mli_package/include/api \
-I./tensorflow/lite/micro/tools/make/downloads/kissfft \
-DSCRATCH_MEM_Z_SIZE=0x10000 \


CCFLAGS+= \
-mcpu=em4_fpus \
-mlittle-endian \
-mcode-density \
-mdiv-rem \
-mswap \
-mnorm \
-mmpy-option=6 \
-mbarrel-shifter \
-mfpu=fpus_all \
-fno-unwind-tables \
-ffunction-sections \
-fdata-sections \
-fmessage-length=0 \
-DTF_LITE_STATIC_MEMORY \
-DTF_LITE_DISABLE_X86_NEON \
-O3 \
-DREDUCE_CODESIZE \
-mxy \
-include $(SDK_PATH)/core_config.h \
-I. \
-I./$(SDK_PATH) \
-I./$(LIB_PATH) \
-I./third_party/gemmlowp \
-I./third_party/flatbuffers/include \
-I./third_party/ruy \
-I./third_party/arc_mli_package/include \
-I./third_party/arc_mli_package/include/api \
-I./tensorflow/lite/micro/tools/make/downloads/kissfft \

LDFLAGS +=  -Wl,-lmli -Wl,-lmwdepend -Wl,-marcv2elfx -Wl,-Map=memory.map -Wl,--strip-debug -Wl,--stats,--gc-sections -Wl,--cref \
-L$(MLI_PATH) \
-L$(DEPEND_PATH) \
-L. \
-Wl,--start-group \
$(SDK_PATH)/libboard_open_socket.a \
$(SDK_PATH)/libcpuarc.a \
$(SDK_PATH)/libbss.a \
$(SDK_PATH)/libboard_socket.a \
$(SDK_PATH)/liblibcommon.a \
$(SDK_PATH)/liblibaudio.a \
$(SDK_PATH)/liblibsecurity.a \
$(SDK_PATH)/liblibsensordp.a \
$(SDK_PATH)/liblibtflm.a \
-Wl,--end-group \

endif # ARC_TOOLCHAIN


#=============================================================
# Common rules
#=============================================================
.PHONY: all qwiic_ccs811_bme280 qwiic_ms8607

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(EXT_CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(EXT_CFLAGS) $(INCLUDES) -c $< -o $@


#=================================================================
# Global rules
#=================================================================
all: 
	$(error "please specific app=")

smart_agriculture_app: MAP_NAME = smart_agriculure_app
smart_agriculture_app: smart_agriculture_app.elf

smart_agriculture_app.elf : $(LORA_OBJS) ${TFOBJS}
	$(CXX) $(CXXFLAGS) -o $@ $(LORA_OBJS) ${TFOBJS} $(LDFLAGS)

clean:
	@echo 'cleaning'
	-@$(RM) $(qwiic_ccs811_bme280_OBJS) $(qwiic_ms8607_OBJS) ${qwiic_ms8607_sparkfun_OBJS} ${qwiic_ccs811_bme280_sparkfun_OBJS} $(TFOBJS) $(LORA_OBJS)
	-@$(RM) *.elf
	-@$(RM) *.map
	
download: 
	@echo 'downloading'
	@$(DL) $(SDK_LINK)  -o $(SDK_NAME)
	@$(DL) $(TOOL_LINK)  -o $(TOOL_NAME)
	@$(DL) $(DEPEND_LINK)  -o $(DEPEND_NAME)
	@$(DL) $(TF_LINK) -o $(TF_NAME)
	@$(UNZIP) -o $(SDK_NAME) -d $(SDK_LOC) 
	@$(UNZIP) -o $(TOOL_NAME) -d $(TOOL_LOC)	
	@$(UNZIP) -o $(DEPEND_NAME) -d $(DEPEND_LOC)
	@$(UNZIP) -o $(TF_NAME) -d $(TF_LOC)
	@$(RM) $(SDK_NAME)
	@$(RM) $(TOOL_NAME)	
	@$(RM) $(DEPEND_NAME)
	@$(RM) $(TF_NAME)
	@$(CP) "himax_tflm-f4c05b8eee81431292bf1c7e5e7d4e721ad7c201/tensorflow/" "./"
	@$(RM) "himax_tflm-f4c05b8eee81431292bf1c7e5e7d4e721ad7c201"
	@mkdir -p $(MODEL_LOC)
	@$(DL) $(MODEL_LINK)  -o $(MODEL_NAME)
	@$(UNZIP) -o $(MODEL_NAME) -d $(MODEL_LOC)
	@$(RM) $(MODEL_NAME)
	@$(DL) $(LIB_LINK)  -o $(LIB_NAME)  
	@$(UNZIP) -o $(LIB_NAME) -d $(LIB_LOC)
	@$(RM) $(LIB_NAME)
	

ifeq ($(ARC_TOOLCHAIN), mwdt)
flash:
ifdef app
	@export PATH=$(shell pwd)/$(GEN_TOOL_DIR)/:$$PATH && \
	cp $(app).elf $(app).map $(GEN_TOOL_DIR) && \
	cd $(GEN_TOOL_DIR) && \
	$(GEN_TOOL_NAME) -e $(app).elf -m $(app).map -o $(app).img -s 1024 && \
	cp $(app)*.img .. && \
	rm $(app).elf $(app).map $(app)*.img
else
	$(error "please specific app=")
endif
else ifeq ($(ARC_TOOLCHAIN), gnu)
flash:
ifdef app
	@export PATH=$(shell pwd)/$(GEN_TOOL_DIR)/:$$PATH && \
	cp $(app).elf $(GEN_TOOL_DIR) && \
	cd $(GEN_TOOL_DIR) && \
	$(GEN_TOOL_NAME) -e $(app).elf -s 1024 -o $(app).img && \
	cp $(app)*.img .. && \
	rm $(app).elf $(app)*.img
else
	$(error "please specific app=")
endif

endif 	
