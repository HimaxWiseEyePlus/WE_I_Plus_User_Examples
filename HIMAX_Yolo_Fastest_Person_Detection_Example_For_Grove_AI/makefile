##
# Application makefile
##


RM = rm -rf
DL = curl -Ls -f
UNZIP = unzip 

LIB_LINK = https://www.himax.com.tw/we-i/grove_ai/third_party_grove_ai_v01.zip
LIB_LOC = .
LIB_NAME = third_party_grove_ai_v01.zip


TFLM_LIB_LINK = https://www.himax.com.tw/we-i/grove_ai/tensorflow_grove_ai_v01.zip
TFLM_LIB_LOC = .
TFLM_LIB_NAME = tensorflow_grove_ai_v01.zip

SDK_LINK = https://www.himax.com.tw/we-i/grove_ai/himax_we1_sdk_grove_ai_v01.zip
SDK_LOC = .
SDK_NAME = himax_we1_sdk_grove_ai_v01.zip

TOOL_LINK = https://www.himax.com.tw/we-i/grove_ai/tools_grove_ai_v01.zip
TOOL_LOC = .
TOOL_NAME = tools_grove_ai_v01.zip

YOLO_HIMAX_LINK = https://github.com/HimaxWiseEyePlus/Yolo-Fastest/raw/master/ModelZoo/yolo-fastest-1.1_160_person/yolo-fastest-1_1_160_person_himax.tflite
YOLO_HIMAX_LOC = .
YOLO_HIMAX_NAME = yolo_himax.tflite

download:
	@echo 'downloading'
	@$(DL) $(YOLO_HIMAX_LINK) -o $(YOLO_HIMAX_NAME) 
	@$(DL) $(TFLM_LIB_LINK)  -o $(TFLM_LIB_NAME) 
	@$(DL) $(SDK_LINK)  -o $(SDK_NAME)
	@$(DL) $(TOOL_LINK)  -o $(TOOL_NAME)
	@$(DL) $(LIB_LINK)  -o $(LIB_NAME) 
	@$(UNZIP) -o $(LIB_NAME) -d $(LIB_LOC)
	@$(UNZIP) -o $(TFLM_LIB_NAME) -d $(TFLM_LIB_LOC)
	@$(UNZIP) -o $(SDK_NAME) -d $(SDK_LOC) 
	@$(UNZIP) -o $(TOOL_NAME) -d $(TOOL_LOC)
	@$(RM) $(TFLM_LIB_NAME)
	@$(RM) $(LIB_NAME)
	@$(RM) $(SDK_NAME)
	@$(RM) $(TOOL_NAME)

##
# root dir of embARC
##
EMBARC_ROOT = .

# USING Pre-built CV library
# 1 for enable
# other for disable
##
LIB_CV_PREBUILT = 0

APP_TYPE = grove_ai_family_yolo

# application makefile
include $(EMBARC_ROOT)/grove_ai_family_yolo/grove_ai_family.mk

# include current project makefile
COMMON_COMPILE_PREREQUISITES += makefile

### Options above must be added before include options.mk ###
# include key embARC build system makefile
override EMBARC_ROOT := $(strip $(subst \,/,$(EMBARC_ROOT)))
-include $(EMBARC_ROOT)/himax_we1_sdk/options/options.mk
