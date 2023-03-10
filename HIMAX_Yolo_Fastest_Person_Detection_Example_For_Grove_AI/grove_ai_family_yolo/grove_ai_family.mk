# application root declaration
SCENARIO_APP_ROOT = $(EMBARC_ROOT)/

#boards
APPL_DEFINES += -DGROVE_VISION_AI

#user define linker script
USER_LINKER_SCRIPT_FILE = $(EMBARC_ROOT)/grove_ai_family_yolo/grove_ai_family.ld

#if you don't use the OV camera, pls comment this code
APPL_DEFINES += -DCIS_OV_SENSOR -DCIS_OV2640_BAYER

LIB_SEL += tflitemicro_25

SCENARIO_APP_SUPPORT_LIST +=	grove_ai_family_yolo/configs \
								grove_ai_family_yolo/debugger \
								grove_ai_family_yolo/communication \
								grove_ai_family_yolo/communication/webusb \
								grove_ai_family_yolo/datapath \
								grove_ai_family_yolo/yolo_fastest \
								grove_ai_family_yolo/drivers \
								grove_ai_family_yolo/drivers/flash \
								grove_ai_family_yolo/drivers/sensor \
								grove_ai_family_yolo/drivers/sensor/camera \
					
SCENARIO_APP_INCDIR = $(EMBARC_ROOT)/grove_ai_family_yolo

SCENARIO_APP_SUPPORT_SORTED = $(sort $(SCENARIO_APP_SUPPORT_LIST))		
SCENARIO_APP_INCDIR += $(foreach SCENARIO_APP_CLI, $(SCENARIO_APP_SUPPORT_SORTED), $(addprefix $(SCENARIO_APP_ROOT), $(SCENARIO_APP_CLI)))

SCENARIO_APP_CSRCS = $(call get_csrcs, $(SCENARIO_APP_INCDIR))
SCENARIO_APP_CXXSRCS = $(call get_cxxsrcs, $(SCENARIO_APP_INCDIR))
SCENARIO_APP_CCSRCS = $(call get_ccsrcs, $(SCENARIO_APP_INCDIR))
SCENARIO_APP_ASMSRCS = $(call get_asmsrcs, $(SCENARIO_APP_ASMSRCDIR))
