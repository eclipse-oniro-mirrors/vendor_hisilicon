# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.

ifeq ($(filter $(LITEOSTOPDIR)/config.mk,$(MAKEFILE_LIST)),)
include $(LITEOSTOPDIR)/config.mk
endif

MODULE_NAME := hdf_test_macro
LOCAL_HCS_ROOT := $(shell pwd)

HDF_FRAMWORK_ROOT = $(abspath $(LITEOSTOPDIR)/../../drivers/hdf_core/framework)
HC_GEN_DIR = $(HDF_FRAMWORK_ROOT)/tools/hc-gen
HC_GEN = $(OUT)/hc_gen_build/hc-gen

HDF_FRAMWORK_TEST_ROOT = $(HDF_FRAMWORK_ROOT)/test/unittest/utils/hcs_parser/unittest
HCS_MACRO_OBJ := hcs_macro_cases.o
HCS_MACRO_SRC := $(HDF_FRAMWORK_TEST_ROOT)/hcs_macro_cases.c
HCS_DEP := $(HDF_FRAMWORK_TEST_ROOT)/hdf_macro_test.h
HCS_MACRO_GEN_FILE := $(HDF_FRAMWORK_TEST_ROOT)/hdf_macro_test
HCS_FILE := $(PRODUCT_PATH)/hdf_config/hdf_test/hdf_test.hcs

$(warning HC_GEN = $(HC_GEN))
$(warning HCS_MACRO_SRC = $(HCS_MACRO_SRC))
$(warning HCS_MACRO_GEN_FILE = $(HCS_MACRO_GEN_FILE))

$(HCS_MACRO_OBJ): $(HCS_MACRO_SRC) $(HCS_DEP)
	$(Q)$(CC) $(CFLAGS) $(LOCAL_FLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

$(HCS_DEP): $(HC_GEN)
	$(Q)echo gen hdf built-in config macro
	$(Q)$(HC_GEN) -m -o $(HCS_MACRO_GEN_FILE) $(HCS_FILE)

$(HC_GEN):
	$(HIDE)make -j -C $(HC_GEN_DIR) BUILD_DIR=$(dir $@)

$(OUT)/lib/lib$(MODULE_NAME).a: $(HCS_MACRO_OBJ)
	$(HIDE)$(AR) $(ARFLAGS) $@ $^
	
all : $(OUT)/lib/lib$(MODULE_NAME).a

.PHONY: all