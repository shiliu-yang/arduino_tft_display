#include "app_config.h"

#if ENABLE_APP_DISPLAY

#include "app_display.h"

#include <SPI.h>
#include <TFT_eSPI.h>

#include "TuyaIoT.h"
#include "TuyaIoTWeather.h"
#include <Log.h>

#include "image_cloudy.h"
#include "image_light_rain.h"
#include "image_sun.h"
// #include "image_overcast_sky.h"

#include "image_temp.h"
#include "image_humi.h"

#include "image_turn_on.h"
#include "image_turn_off.h"

// font
// #include "GoogleSans_Regular16.h"
#include "GoogleSans_Regular18.h"
// #include "GoogleSans_Regular24.h"
// #include "GoogleSans_Regular28.h"
#include "GoogleSans_Regular32.h"

#include "tal_time_service.h"
#include "tal_workq_service.h"
#include "cJSON.h"

#define DISPLAY_PAGE_WEATHER  0
#define DISPLAY_PAGE_SWITCH   1
#define DISPLAY_PAGE_CLOCK    2
#define DISPLAY_PAGE_MAX      3

// weather data
#define WEATHER_UPDATE_SEC  (10*60)

// RGB 40, 52, 65
#define SLATE_BLUE          (0x29A8)
// RGB 255，255，224
#define LIGHT_YELLOW        (0xFFFC)

#define WEATHER_BACKGROUND  SLATE_BLUE

#define SWITCH_BACKGROUND   LIGHT_YELLOW

struct weather_data {
  uint32_t weather_index;
  int8_t cur_temp_high;
  int8_t cur_temp_low;
  int8_t real_feel;
  int8_t cur_humi;
};
static struct weather_data sg_last_weather_data = {0} ;
static struct weather_data sg_weather_data = {
  .weather_index = TW_WEATHER_SUNNY,
  .cur_temp_high = 26,
  .cur_temp_low = 19,
  .real_feel = 23,
  .cur_humi = 62,
};

// work queue
static DELAYED_WORK_HANDLE sg_workq_handle = NULL;

TFT_eSPI tft = TFT_eSPI();

TuyaIoTWeatherClass TuyaWeather;

static void weatherUpdate(void)
{
  int rt = OPRT_OK;

  PR_DEBUG("---> weather update");

  int high = 0, low = 0;
  rt = TuyaWeather.getTempHighLow(high, low);
  if (OPRT_OK == rt) {
    PR_DEBUG("high: %d, low: %d", high, low);
    sg_weather_data.cur_temp_high = high;
    sg_weather_data.cur_temp_low = low;
  }

  String weather = TuyaWeather.get(0x0001);
  PR_DEBUG("weather: %s", weather.c_str());

  cJSON *root = cJSON_Parse(weather.c_str());
  if (!root) {
    PR_ERR("cJSON_Parse fail");
    return;
  }

  cJSON *dataObj = cJSON_GetObjectItem(root, "data");
  if (dataObj) {
    cJSON *tempObj = cJSON_GetObjectItem(dataObj, "w.temp");
    if (tempObj) {
      int temp = tempObj->valueint;
      PR_DEBUG("Temperature: %d\n", temp);
    }

    cJSON *humidityObj = cJSON_GetObjectItem(dataObj, "w.humidity");
    if (humidityObj) {
      int humidity = humidityObj->valueint;
      PR_DEBUG("Humidity: %d\n", humidity);
      sg_weather_data.cur_humi = humidity;
    }

    cJSON *realFeelObj = cJSON_GetObjectItem(dataObj, "w.realFeel");
    if (realFeelObj) {
      int realFeel = realFeelObj->valueint;
      PR_DEBUG("Real Feel: %d\n", realFeel);
      sg_weather_data.real_feel = realFeel;
    }

    cJSON *pm25Obj = cJSON_GetObjectItem(dataObj, "w.pm25");
    if (pm25Obj) {
      int pm25 = pm25Obj->valueint;
      PR_DEBUG("PM2.5: %d\n", pm25);
    }

    cJSON *conditionNumObj = cJSON_GetObjectItem(dataObj, "w.conditionNum");
    if (conditionNumObj) {
      const char *conditionNum = conditionNumObj->valuestring;
      PR_DEBUG("Condition Number: %s\n", conditionNum);
      String w_index = String(conditionNum);
      sg_weather_data.weather_index = w_index.toInt();
    }

    cJSON *cityObj = cJSON_GetObjectItem(dataObj, "c.city");
    if (cityObj) {
      const char *city = cityObj->valuestring;
      PR_DEBUG("City: %s\n", city);
    }
  }

  cJSON *expirationObj = cJSON_GetObjectItem(root, "expiration");
  if (expirationObj) {
    int expiration = expirationObj->valueint;
    PR_DEBUG("Expiration: %d\n", expiration);
  }

  cJSON_Delete(root);

  return;
}

static void _display_weather_icon(uint32_t index)
{
  const uint16_t *p_icon = NULL;

  switch (index) {
    case TW_WEATHER_SUNNY:
    case TW_WEATHER_MOSTLY_CLEAR:
    case TW_WEATHER_CLEAR:
    {
      p_icon = image_sun;
    } break;
    case TW_WEATHER_ISOLATED_SHOWER:
    case TW_WEATHER_LIGHT_SHOWER:
    case TW_WEATHER_LIGHT_TO_MODERATE_RAIN:
    case TW_WEATHER_SHOWER:
    case TW_WEATHER_LIGHT_RAIN:
    case TW_WEATHER_MODERATE_RAIN:
    {
      p_icon = image_light_rain;
    } break;
    case TW_WEATHER_PARTLY_CLOUDY:
    case TW_WEATHER_CLOUDY:
    case TW_WEATHER_OVERCAST:
    {
      p_icon = image_cloudy;
    } break;
    default : return;
  }

  tft.setSwapBytes(true);
  uint16_t transp = 0x0000;
  tft.fillRect(20, 10, 96, 96, WEATHER_BACKGROUND);
  tft.pushImage(20, 10, 96, 96, (uint16_t *)p_icon, transp);
}

static void _display_real_feel(int8_t temp)
{
  tft.setTextColor(TFT_WHITE);
  tft.loadFont(GoogleSans_Regular32);
  tft.setCursor(135, 50);
  char realFellString[6] = {0};
  snprintf(realFellString, 6, "%d°C", temp);
  // tft.fillRect(135, 50, 96, 96, WEATHER_BACKGROUND);
  tft.fillRect(135, 50, 240-135, 32, WEATHER_BACKGROUND);
  tft.print(realFellString);
  tft.unloadFont();
}

static void _display_temp(int8_t min, int8_t max)
{
  tft.setSwapBytes(true);
  uint16_t transp = 0x0000;
  tft.pushImage(15, 140, 36, 36, (uint16_t *)image_temp, transp);

  tft.setTextColor(TFT_WHITE);
  tft.loadFont(GoogleSans_Regular18);
  tft.setCursor(66, 140+12);
  char tempString[16] = {0};
  snprintf(tempString, 16, "%d°C ~ %d°C", min, max);
  tft.fillRect(66, 140+12, 240-66, 18, WEATHER_BACKGROUND);
  tft.print(tempString);
  tft.unloadFont();
}

static void _display_humi(int8_t humi)
{
  tft.setSwapBytes(true);
  uint16_t transp = 0x0000;
  tft.pushImage(20, 140 + 36 + 20, 36, 36, (uint16_t *)image_humi, transp);

  tft.setTextColor(TFT_WHITE);
  tft.loadFont(GoogleSans_Regular18);
  tft.setCursor(20 + 36 + 10, 140 + 36 + 20+12);
  char humiString[8] = {0};
  snprintf(humiString, 10, "%d%%", humi);
  tft.fillRect(66, 140 + 36 + 20+12, 240-66, 18, WEATHER_BACKGROUND);
  tft.print(humiString);
  tft.unloadFont();
}

static void _display_screen_onoff(uint8_t is_first, uint8_t onoff)
{
  static uint8_t _last_on_off = 0xff;

  onoff = onoff ? 1 : 0;

  if (is_first) {
    tft.fillScreen(SWITCH_BACKGROUND);
    _last_on_off = 0xff;
  }

  const uint16_t *p_icon = onoff ? (image_turn_on) : (image_turn_off);

  if (onoff == _last_on_off) {
    return;
  }
  _last_on_off = onoff;

  tft.setSwapBytes(true);
  uint16_t transp = 0x0000;
  tft.fillRect(80, 80, 80, 80, SWITCH_BACKGROUND);
  tft.pushImage(80, 80, 80, 80, (uint16_t *)p_icon, transp);

  return;
}

static void _app_display_weather(uint8_t is_first)
{
  static uint32_t update_cnt = 0;

  if (is_first) {
    tft.fillScreen(WEATHER_BACKGROUND);

    sg_last_weather_data.weather_index = 0xffffffff;
    sg_last_weather_data.real_feel = 0xff;
    sg_last_weather_data.cur_temp_low = 0xff;
    sg_last_weather_data.cur_temp_high = 0xff;
    sg_last_weather_data.cur_humi = 0xff;
  }

  // if (OPRT_OK != tal_time_check_time_sync()) {
  //   PR_DEBUG("time not sync");
  //   return;
  // }

  // // network check
  // if (false == TuyaIoT.networkCheck()) {
  //   PR_DEBUG("network not connect");
  //   return;
  // }

  // TODO: add last update time
  if (OPRT_OK == tal_time_check_time_sync() && TuyaIoT.networkCheck()) {
    if (0 == update_cnt) {
      weatherUpdate();
    }
    update_cnt = (update_cnt + 1) % WEATHER_UPDATE_SEC;
  }

  // if (is_first) {
  //   sg_last_weather_data.weather_index = sg_weather_data.weather_index;
  //   _display_weather_icon(sg_weather_data.weather_index);

  //   sg_last_weather_data.real_feel = sg_weather_data.real_feel;
  //   _display_real_feel(sg_weather_data.real_feel);

  //   sg_last_weather_data.cur_temp_low = sg_weather_data.cur_temp_low;
  //   sg_last_weather_data.cur_temp_high = sg_weather_data.cur_temp_high;
  //   _display_temp(sg_weather_data.cur_temp_low, sg_weather_data.cur_temp_high);

  //   sg_last_weather_data.cur_humi = sg_weather_data.cur_humi;
  //   _display_humi(sg_weather_data.cur_humi);

  //   return;
  // }

  // weather icon
  if (sg_weather_data.weather_index != sg_last_weather_data.weather_index) {
    sg_last_weather_data.weather_index = sg_weather_data.weather_index;
    _display_weather_icon(sg_weather_data.weather_index);
  }

  // real feel
  if (sg_weather_data.real_feel != sg_last_weather_data.real_feel) {
    sg_last_weather_data.real_feel = sg_weather_data.real_feel;
    _display_real_feel(sg_weather_data.real_feel);
  }

  // today temperature range
  if (sg_weather_data.cur_temp_low != sg_last_weather_data.cur_temp_low ||
      sg_weather_data.cur_temp_high != sg_last_weather_data.cur_temp_high) {
    sg_last_weather_data.cur_temp_low = sg_weather_data.cur_temp_low;
    sg_last_weather_data.cur_temp_high = sg_weather_data.cur_temp_high;
    _display_temp(sg_weather_data.cur_temp_low, sg_weather_data.cur_temp_high);
  }

  // humidity
  if (sg_weather_data.cur_humi != sg_last_weather_data.cur_humi) {
    sg_last_weather_data.cur_humi = sg_weather_data.cur_humi;
    _display_humi(sg_weather_data.cur_humi);
  }
}

static uint8_t sg_display_page = 0;

static uint8_t sg_onoff = 0;

static void _app_display_refresh(void *data)
{
  uint8_t is_first = 0;
  static uint8_t sg_last_page = 0xff;

  if (sg_last_page != sg_display_page) {
    sg_last_page = sg_display_page;
    is_first = 1;
  }

  switch (sg_display_page) {
    case DISPLAY_PAGE_WEATHER:
      _app_display_weather(is_first);
    break;
    case DISPLAY_PAGE_SWITCH:
      _display_screen_onoff(is_first, sg_onoff);
    break;
    case DISPLAY_PAGE_CLOCK:
      app_display_colck(is_first);
    break;
    default : break;
  }
}

void app_display_init(void)
{
  int rt = OPRT_OK;

  tft.init();
  tft.setRotation(0);

  sg_display_page = DISPLAY_PAGE_WEATHER;

#if 1
  // _display_weather_icon(sg_weather_data.weather_index);
  // _display_real_feel(sg_weather_data.real_feel);
  // _display_temp(sg_weather_data.cur_temp_low, sg_weather_data.cur_temp_high);
  // _display_humi(sg_weather_data.cur_humi);
  // sg_display_page = 0;
#else
  // _display_screen_onoff(0);
  // sg_display_page = 1;

  // DISPLAY_PAGE_CLOCK

  sg_display_page = DISPLAY_PAGE_CLOCK;
#endif

  rt = tal_workq_init_delayed(WORKQ_SYSTEM, _app_display_refresh, NULL, &sg_workq_handle);
  if (OPRT_OK != rt) {
    return;
  }

  tal_workq_start_delayed(sg_workq_handle, 1*1000, LOOP_CYCLE);

}

void app_display_weather_icon_set(uint32_t index)
{
  sg_weather_data.weather_index = index;
}

void app_display_weather_real_feel_set(int8_t temp)
{
  sg_weather_data.real_feel = temp;
}

void app_display_weather_cur_temp_high_set(int8_t temp)
{
  sg_weather_data.cur_temp_high = temp;
}

void app_display_weather_cur_temp_low_set(int8_t temp)
{
  sg_weather_data.cur_temp_low = temp;
}

void app_display_weather_cur_humi_set(int8_t humi)
{
  sg_weather_data.cur_humi = humi;
}

void app_display_onoff_set(uint8_t onoff)
{
  sg_onoff = onoff;
}

uint8_t app_display_onoff_get(void)
{
  return sg_onoff;
}

void app_display_page_change(void)
{
  sg_display_page++;
  sg_display_page = sg_display_page % DISPLAY_PAGE_MAX;
}

#endif
