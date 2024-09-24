#include "app_config.h"

#include "Log.h"
#include "Ticker.h"

#include "tal_memory.h"

#include "app_tuya_iot.h"
#include "app_button.h"

#if ENABLE_APP_DISPLAY
#include "app_display.h"
#endif

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
}

void loop() {
  // put your main code here, to run repeatedly:
  app_button_loop();
  delay(5);
}
