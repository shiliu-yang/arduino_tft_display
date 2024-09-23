#include "app_button.h"

#include "OneButton.h"
#include "TuyaIoT.h"

#include "app_display.h"
#include "app_tuya_iot.h"

#define buttonPin BUTTON_BUILTIN
OneButton button(buttonPin);

static void buttonClick()
{
  Serial.println("Button clicked");
  uint8_t status = app_display_onoff_get();
  app_display_onoff_set(!status);
  TuyaIoT.write(DPID_SWITCH, !status);
}

static void buttonDoubleClick()
{
  Serial.println("Button double clicked");
  app_display_page_change();
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
