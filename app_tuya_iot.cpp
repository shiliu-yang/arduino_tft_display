#include "app_config.h"

#include "app_tuya_iot.h"

#include "TuyaIoT.h"

#if ENABLE_APP_DISPLAY
#include "app_display.h"
#endif

// Tuya license
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

static void tuyaIoTEventCallback(tuya_event_msg_t *event)
{
  int ledState = 0;

  tuya_event_id_t event_id = TuyaIoT.eventGetId(event);

  switch (event_id) {
    case TUYA_EVENT_BIND_START: {
      Serial.println("---> TUYA_EVENT_BIND_START");
    } break;
    case TUYA_EVENT_ACTIVATE_SUCCESSED: {
      Serial.println("---> TUYA_EVENT_ACTIVATE_SUCCESSED");
    } break;
    case TUYA_EVENT_MQTT_CONNECTED: {
      // Update all DP
      Serial.println("---> TUYA_EVENT_MQTT_CONNECTED");
    } break;
    case TUYA_EVENT_TIMESTAMP_SYNC: {
      tal_time_set_posix(event->value.asInteger, 1);
    } break;
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
      Serial.println("---> TUYA_EVENT_DP_RECEIVE_OBJ");
      uint16_t dpNum = TuyaIoT.eventGetDpNum(event);
      for (uint16_t i = 0; i < dpNum; i++) {
        uint8_t dpid = TuyaIoT.eventGetDpId(event, i);
        switch (dpid) {
          case DPID_SWITCH: {
            TuyaIoT.read(event, DPID_SWITCH, ledState);
            Serial.print("Receive DPID_SWITCH: "); Serial.println(ledState);
#if ENABLE_APP_DISPLAY
            app_display_onoff_set(ledState);
#endif
            TuyaIoT.write(DPID_SWITCH, ledState);
          } break;
          default : break;
        }
      }
    } break;
    default: break;
  }
}

void app_iot_init(void)
{
  // license
  tuya_iot_license_t license;
  int rt = TuyaIoT.readBoardLicense(&license);
  if (OPRT_OK != rt) {
    license.uuid = TUYA_DEVICE_UUID;
    license.authkey = TUYA_DEVICE_AUTHKEY;
    Serial.println("Replace the TUYA_DEVICE_UUID and TUYA_DEVICE_AUTHKEY contents, otherwise the demo cannot work");
  }
  Serial.print("uuid: "); Serial.println(license.uuid);
  Serial.print("authkey: "); Serial.println(license.authkey);
  TuyaIoT.setLicense(license.uuid, license.authkey);

  TuyaIoT.setEventCallback(tuyaIoTEventCallback);
  // The "PROJECT_VERSION" comes from the "PROJECT_VERSION" field in "appConfig.json"
  TuyaIoT.begin("qhivvyqawogv04e4", PROJECT_VERSION);
}
