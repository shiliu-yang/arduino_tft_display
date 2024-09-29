#include "app_config.h"

#include "app_weather.h"
#include "TuyaIoT.h"
#include "TuyaIoTWeather.h"
#include "Log.h"

#include "tal_workq_service.h"

#include "SmallDesktopDisplay.h"

DELAYED_WORK_HANDLE weatherWorkQueueHandle;

TuyaIoTWeatherClass TuyaWeather;

TIME_MS TmInterval;

void weatherWorkQueueCallback(void *data)
{
  int rt = 0;

  static int nextUpdateCount = 10, cnt = 0;

  struct SmallDesktopWeather weatherData;

  int weather, temp, humi, realFeel, mbar, uvi, highTemp, lowTemp, windLevel;
  int aqi, qualityLevel, pm25, pm10, o3, no2, co, so2;
  String windDir, windSpeed;

  cnt++;

  if (false == TuyaIoT.isTimeSync()) {
    cnt = 0;
    return;
  }

  if (nextUpdateCount > cnt) {
    return;
  }

  memset(&weatherData, 0, sizeof(struct SmallDesktopWeather));

  rt = TuyaWeather.getCurrentConditions(weather, temp, humi, realFeel, mbar, uvi);
  if (rt != OPRT_OK) {
    PR_ERR("Get current conditions failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.weatherCode = weather;
  weatherData.realFeel = realFeel;
  weatherData.humidity = humi;

  rt = TuyaWeather.getTodayHighLowTemp(highTemp, lowTemp);
  if (rt != OPRT_OK) {
    PR_ERR("Get today high low temp failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.highTemp = highTemp;
  weatherData.lowTemp = lowTemp;

  rt = TuyaWeather.getCurrentAQI(aqi, qualityLevel, pm25, pm10, o3, no2, co, so2);
  if (rt != OPRT_OK) {
    PR_ERR("Get current AQI failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.aqi = aqi;

  rt = TuyaWeather.getCurrentWindCN(windDir, windSpeed, windLevel);
  if (rt != OPRT_OK) {
    PR_ERR("Get current wind failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.windDir = windDir;
  weatherData.windLevel = windLevel;

  String province, city, area;
  rt = TuyaWeather.getCity(province, city, area);
  if (rt != OPRT_OK) {
    PR_ERR("Get city name failed");

    nextUpdateCount += 5;
    nextUpdateCount = (nextUpdateCount >= 30) ? 30 : nextUpdateCount;
    cnt = 0;
    return;
  }
  weatherData.cityName = city;

  SmallDesktopWeatherSet(weatherData);

  // Serial.println("Weather data updated");
  // Serial.println("Weather code: " + String(weatherData.weatherCode));
  // Serial.println("Real feel: " + String(weatherData.realFeel));
  // Serial.println("Humidity: " + String(weatherData.humidity));
  // Serial.println("AQI: " + String(weatherData.aqi));
  // Serial.println("High temp: " + String(weatherData.highTemp));
  // Serial.println("Low temp: " + String(weatherData.lowTemp));
  // Serial.println("City name: " + weatherData.cityName);
  // Serial.println("Wind dir: " + weatherData.windDir);
  // Serial.println("Wind level: " + String(weatherData.windLevel));

  nextUpdateCount = 15*60; // 15 minutes
  cnt = 0;
}

void app_weather_init(void)
{
  TmInterval = 1000;

  tal_workq_init_delayed(WORKQ_SYSTEM, weatherWorkQueueCallback, NULL, &weatherWorkQueueHandle);
  tal_workq_start_delayed(weatherWorkQueueHandle, TmInterval, LOOP_CYCLE);
}

