# Copyright (c) 2025 HPMicro
# SPDX-License-Identifier: Apache-2.0

# generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hpm6e_ecat_io)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bsp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ssc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/port
  ${HPM_SDK_DIR}/components/eeprom_emulation
  ${HPM_SDK_DIR}/components/eeprom_emulation/port
  ${HPM_SDK_DIR}/utils
)

target_sources(app PRIVATE
  src/main.c
  src/bsp/ecat_zephyr_port.c
  # hpm_sdk EtherCAT port layer
  src/port/hpm_ecat_hw.c
  src/port/hpm_ecat_phy.c
  src/port/hpm_ecat_e2p_emulation.c
  # SSC application layer (generated + customized)
  src/app_layer/app_ethercat.c
  # Beckhoff SSC stack (generated)
  src/ssc/coeappl.c
  src/ssc/ecatappl.c
  src/ssc/ecatcoe.c
  src/ssc/ecatslv.c
  src/ssc/mailbox.c
  src/ssc/objdef.c
  src/ssc/sdoserv.c
  # hpm_sdk drivers not compiled by glue (no HAS_HPMSDK_ESC Kconfig);
  # hpm_gptmr_drv.c is already compiled into the zephyr library by glue
  ${HPM_SDK_DIR}/drivers/src/hpm_esc_drv.c
  # flash eeprom emulation component
  ${HPM_SDK_DIR}/components/eeprom_emulation/eeprom_emulation.c
  ${HPM_SDK_DIR}/components/eeprom_emulation/port/hpm_nor_flash.c
  ${HPM_SDK_DIR}/utils/hpm_crc32.c
)
