@echo off
cd /D "D:\ESP32_Workspace\My projects\ESP32 GPS Loger\Parts projects\6. Web server\4. Simple HTTP server AJAX\build\esp-idf\esp_system" || (set FAIL_LINE=2& goto :ABORT)
C:\Espressif\python_env\idf5.2_py3.11_env\Scripts\python.exe C:/Espressif/frameworks/esp-idf-v5.2.2/tools/ldgen/ldgen.py --config "D:/ESP32_Workspace/My projects/ESP32 GPS Loger/Parts projects/6. Web server/4. Simple HTTP server AJAX/sdkconfig" --fragments-list C:/Espressif/frameworks/esp-idf-v5.2.2/components/xtensa/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_ringbuf/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_mm/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/gpio/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/gptimer/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/i2c/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/ledc/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/mcpwm/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/rmt/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/twai/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/driver/uart/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_pm/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/spi_flash/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_system/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_system/app.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_rom/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/hal/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/log/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/heap/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/soc/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_hw_support/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_hw_support/dma/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/freertos/linker_common.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/freertos/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/newlib/newlib.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/newlib/system_libs.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_common/common.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_common/soc.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/app_trace/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_event/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_phy/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/vfs/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/lwip/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_netif/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/wpa_supplicant/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_wifi/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_adc/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_eth/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_gdbstub/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_psram/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/esp_lcd/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/espcoredump/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/ieee802154/linker.lf;C:/Espressif/frameworks/esp-idf-v5.2.2/components/openthread/linker.lf --input "D:/ESP32_Workspace/My projects/ESP32 GPS Loger/Parts projects/6. Web server/4. Simple HTTP server AJAX/build/esp-idf/esp_system/ld/sections.ld.in" --output "D:/ESP32_Workspace/My projects/ESP32 GPS Loger/Parts projects/6. Web server/4. Simple HTTP server AJAX/build/esp-idf/esp_system/ld/sections.ld" --kconfig C:/Espressif/frameworks/esp-idf-v5.2.2/Kconfig --env-file "D:/ESP32_Workspace/My projects/ESP32 GPS Loger/Parts projects/6. Web server/4. Simple HTTP server AJAX/build/config.env" --libraries-file "D:/ESP32_Workspace/My projects/ESP32 GPS Loger/Parts projects/6. Web server/4. Simple HTTP server AJAX/build/ldgen_libraries" --objdump C:/Espressif/tools/xtensa-esp-elf/esp-13.2.0_20230928/xtensa-esp-elf/bin/xtensa-esp32-elf-objdump.exe || (set FAIL_LINE=3& goto :ABORT)
goto :EOF

:ABORT
set ERROR_CODE=%ERRORLEVEL%
echo Batch file failed at line %FAIL_LINE% with errorcode %ERRORLEVEL%
exit /b %ERROR_CODE%