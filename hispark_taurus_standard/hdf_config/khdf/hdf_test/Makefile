#
# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
#

ccflags-$(CONFIG_DRIVERS_HDF_TEST) +=  -I$(srctree)/include/hdf \
    -I$(srctree)/include/hdf/osal \
    -I$(srctree)/include/hdf/utils \
    -I$(srctree)/drivers/hdf/khdf/osal/include \
    -I$(srctree)/drivers/hdf/framework/utils/include

ifeq ($(LOCAL_HCS_ROOT),)
  LOCAL_HCS_ROOT := $(PRODUCT_PATH)
endif

CURRENT_DIR := $(abspath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
SOURCE_ROOT := $(abspath $(CURRENT_DIR)/../../../../../../)

HC_GEN_DIR := $(abspath $(SOURCE_ROOT)/drivers/hdf_core/framework/tools/hc-gen)
ifneq ($(OUT_DIR),)
HC_GEN := $(OUT_DIR)/kernel/OBJ/${KERNEL_VERSION}/drivers/hdf/khdf/hc_gen_build/hc-gen
else
HC_GEN := $(HC_GEN_DIR)/build/hc-gen
endif
LOCAL_HCS_ROOT := $(CURRENT_DIR)

HCS_DIR := $(LOCAL_HCS_ROOT)

ifneq ($(TARGET_BOARD_PLATFORM),)
  HCS_DIR := $(LOCAL_HCS_ROOT)/$(TARGET_BOARD_PLATFORM)
else
  ifneq ($(CONFIG_ARCH_HI3516DV300),)
    HCS_DIR := $(LOCAL_HCS_ROOT)
  endif
  ifneq ($(CONFIG_ARCH_HI3518EV300),)
    HCS_DIR := $(LOCAL_HCS_ROOT)
  endif
endif
$(info HCS_DIR = $(HCS_DIR))
HCB_FLAGS := -b -i -a

HCS_OBJ := hdf_hcs_hex.o
HCS_OBJ_SRC := $(subst .o,.c,$(notdir $(HCS_OBJ)))

CONFIG_GEN_HEX_SRC := $(addprefix $(LOCAL_HCS_ROOT)/, $(HCS_OBJ_SRC))
CONFIG_HCS_SRC := $(subst _hcs_hex.o,.hcs,$(addprefix $(HCS_DIR)/, $(HCS_OBJ)))

HDF_FRAMWORK_TEST_ROOT = $(abspath $(SOURCE_ROOT)/drivers/hdf_core/framework/test/unittest/utils/hcs_parser/unittest)
HCS_MACRO_OBJ := hcs_macro_cases.o
HCS_MACRO_SRC := $(HDF_FRAMWORK_TEST_ROOT)/hcs_macro_cases.c
HCS_DEP := $(HDF_FRAMWORK_TEST_ROOT)/hdf_macro_test.h
HCS_MACRO_GEN_FILE := $(HDF_FRAMWORK_TEST_ROOT)/hdf_macro_test
HCS_FILE := $(HCS_DIR)/hdf.hcs

$(obj)/$(HCS_OBJ): $(CONFIG_GEN_HEX_SRC)
	$(Q)$(CC) $(c_flags) -c -o $@ $<
	$(Q)rm -f $<

$(CONFIG_GEN_HEX_SRC):  $(LOCAL_HCS_ROOT)/%_hcs_hex.c: $(HCS_DIR)/%.hcs | $(HC_GEN)
	$(Q)echo gen hdf built-in config
	$(Q)if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(Q)$(HC_GEN) $(HCB_FLAGS) -o $(subst _hex.c,,$(@)) $<

$(CONFIG_GEN_SRCS): $(CONFIG_OUT_DIR)%.c: $(HCS_DIR)/%.hcs | $(HC_GEN)
	$(Q)echo gen hdf driver config
	$(Q)if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(Q)$(HC_GEN) -t -o $@ $<

$(HC_GEN):
	$(HIDE)make -C $(HC_GEN_DIR) BUILD_DIR=$(dir $@)

$(obj)/$(HCS_MACRO_OBJ): $(HCS_MACRO_SRC) $(HCS_DEP)
	$(Q)$(CC) $(c_flags) -c -o $@ $<

$(HCS_DEP): $(HC_GEN)
	$(Q)echo gen hdf built-in config macro
	$(Q)$(HC_GEN) -m -o $(HCS_MACRO_GEN_FILE) $(HCS_FILE)

obj-$(CONFIG_DRIVERS_HDF) += $(HCS_OBJ) \
                             $(HCS_MACRO_OBJ)
