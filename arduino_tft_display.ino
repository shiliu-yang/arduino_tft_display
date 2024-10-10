#include "app_config.h"

#include "Log.h"
#include "Ticker.h"

#include "tal_memory.h"

#include "app_tuya_iot.h"
#include "app_button.h"

#if ENABLE_APP_DISPLAY
#include "app_display.h"
#endif

#if ENABLE_SMALL_DESKTOP_DISPLAY
#include "SmallDesktopDisplay.h"
#endif

#include "app_weather.h"

// Print free heap
Ticker heapTicker;

void heapCallback() {
  PR_NOTICE("Free heap: %d", tal_system_get_free_heap_size());
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  heapTicker.attach(5, heapCallback);

  app_iot_init();
  app_button_init();
#if ENABLE_APP_DISPLAY
  app_display_init();
#endif
#if ENABLE_SMALL_DESKTOP_DISPLAY
  SmallDesktopDisplaySetup();
#endif

  app_weather_init(); // 需要 app_iot_init 后调用，app_iot_init 中会进行工作队列初始化
}

uint32_t cnt = 0;

void loop() {
  // put your main code here, to run repeatedly:
  app_button_loop();
  SmallDesktopDisplayLoop();
  delay(10);

  cnt++;
  if (cnt % 100 == 0) {
    Serial.printf("cnt: %d\r\n", cnt);
  }
}
