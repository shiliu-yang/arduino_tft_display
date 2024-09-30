# arduino_tft_display

复刻项目仓库：[chuxin520922/SmallDesktopDisplay (github.com)](https://github.com/chuxin520922/SmallDesktopDisplay/tree/main)

由于 T2 开发板 flash 空间有限，所以天气的图片都存放到了 uf 区域里面。所以在编译烧录主固件前需要先进行一下操作：

+ 擦除应用代码区域，防止当前应用代码有文件系统操作功能。擦除开始地址：0x11000，长度: 0x119000
+ 擦除文件系统区域，***该操作会导致授权信息，激活信息丢失！！！*** 擦除开始地址：0x1d2000，长度: 0x2e000
+ 烧录天气文件写入程序，tools/weatherImgWrite/weatherImgWrite.ino



编译烧录当前固件：

需要提前下载以下 Arduino libraries：

+ [shiliu-yang/TFT_eSPI: Arduino and PlatformIO IDE compatible TFT library optimised for the Raspberry Pi Pico (RP2040), STM32, ESP8266 and ESP32 that supports different driver chips (github.com)](https://github.com/shiliu-yang/TFT_eSPI) 该仓库是从 TFT_eSPI fork 出来，支持 T2 的。
+ [shiliu-yang/arduino-tuyaopen: Arduino core for the T2/T3/T5 (github.com)](https://github.com/shiliu-yang/arduino-tuyaopen) 该仓库针对 TFT_eSPI 进行了 SPI 优化，功能还未 PR 到主仓库。
+ 通过 Arduino IDE 下在 jpg 图片解码库 TJpg_Decoder。
+ 申请一个 tuya open sdk 授权码，在 app_tuya_iot.cpp 文件中进行修改。[tuya-open-sdk-for-device/README_zh.md at master · tuya/tuya-open-sdk-for-device (github.com)](https://github.com/tuya/tuya-open-sdk-for-device/blob/master/README_zh.md#涂鸦云应用) 
+ 编译烧录 arduino_tft_display
