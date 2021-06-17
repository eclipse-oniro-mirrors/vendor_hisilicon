# Copyright (c) 2020 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifneq ($(findstring $(shell uname),Linux),)
HCGEN_PATH := linux-x86/bin/hc-gen
else
HCGEN_PATH := win-x86/bin/hc-gen.exe
endif

ifeq ($(LOCAL_HCS_ROOT),)
LOCAL_HCS_ROOT := $(PRODUCT_PATH)
endif

SOURCE_ROOT:=$(abspath ../../../../)

HC_GEN := hc-gen
BUILD_IN_HC_GEN := $(SOURCE_ROOT)/prebuilts/build-tools/$(HCGEN_PATH)
ifneq ($(wildcard $(BUILD_IN_HC_GEN)),)
HC_GEN := $(BUILD_IN_HC_GEN)
endif

LOCAL_HCS_ROOT := $(abspath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

HCS_DIR := $(LOCAL_HCS_ROOT)

ifneq ($(TARGET_BOARD_PLATFORM),)
HCS_DIR := $(LOCAL_HCS_ROOT)/$(TARGET_BOARD_PLATFORM)
else
ifneq ($(CONFIG_ARCH_HI3516DV300),)
HCS_DIR := $(LOCAL_HCS_ROOT)/hi3516dv300
endif
ifneq ($(CONFIG_ARCH_HI3518EV300),)
HCS_DIR := $(LOCAL_HCS_ROOT)/hi3518ev300
endif
endif
$(info HCS_DIR = $(HCS_DIR))
HCB_FLAGS := -b -i -a

HCS_OBJ := hdf_hcs_hex.o
HCS_OBJ_SRC := $(subst .o,.c,$(notdir $(HCS_OBJ)))

CONFIG_GEN_HEX_SRC := $(addprefix $(LOCAL_HCS_ROOT)/, $(HCS_OBJ_SRC))
CONFIG_HCS_SRC := $(subst _hcs_hex.o,.hcs,$(addprefix $(HCS_DIR)/, $(HCS_OBJ)))

$(obj)/$(HCS_OBJ): $(CONFIG_GEN_HEX_SRC)
	$(Q)$(CC) $(c_flags) -c -o $@ $<
	$(Q)rm -f $<

$(CONFIG_GEN_HEX_SRC):  $(LOCAL_HCS_ROOT)/%_hcs_hex.c: $(HCS_DIR)/%.hcs
	$(Q)echo gen hdf built-in config
	$(Q)if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(Q)$(HC_GEN) $(HCB_FLAGS) -o $(subst _hex.c,,$(@)) $<

$(CONFIG_GEN_SRCS): $(CONFIG_OUT_DIR)%.c: $(HCS_DIR)/%.hcs
	$(Q)echo gen hdf driver config
	$(Q)if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(Q)$(HC_GEN) -t -o $@ $<


obj-$(CONFIG_DRIVERS_HDF) += $(HCS_OBJ)
