#!/bin/sh
"/cygdrive/c/intelFPGA_lite/16.1/quartus/sopc_builder/bin/sopc-create-header-files" \
"./Computer_System.sopcinfo" \
--single arm_a9_hps_0.h \
--module ARM_A9_HPS
