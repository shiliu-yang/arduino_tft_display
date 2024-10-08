#include "app_config.h"

#include "app_button.h"

#include "OneButton.h"
#include "TuyaIoT.h"

#if ENABLE_APP_DISPLAY
#include "app_display.h"
#endif

#include "app_tuya_iot.h"

#define buttonPin BUTTON_BUILTIN
OneButton button(buttonPin);

static void buttonClick()
{
  Serial.println("Button clicked");
#if ENABLE_APP_DISPLAY
  uint8_t status = app_display_onoff_get();
  app_display_onoff_set(!status);
  TuyaIoT.write(DPID_SWITCH, !status);
#endif
}

static void buttonDoubleClick()
{
  Serial.println("Button double clicked");
#if ENABLE_APP_DISPLAY
  app_display_page_change();
#endif
}

static void buttonLongPressStart()
{
  Serial.println("Button long press, remove IoT device.");
  TuyaIoT.remove();
}

void app_button_init(void)
{
  button.attachClick(buttonClick);
  button.attachDoubleClick(buttonDoubleClick);
  button.setPressMs(3*1000);
  button.attachLongPressStart(buttonLongPressStart);
}

void app_button_loop(void)
{
  button.tick();
}
