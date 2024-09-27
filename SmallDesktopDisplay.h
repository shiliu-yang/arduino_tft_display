#pragma once

#include <Arduino.h>

struct SmallDesktopWeather {
  int weatherCode; // 天气代码
  int realFeel; // 体感温度
  int humidity; // 湿度
  int aqi; // 空气质量指数
  int highTemp; // 最高温度
  int lowTemp; // 最低温度
  String cityName; // 城市名称
  String windDir; // 风向
  int windLevel; // 风力等级
};

void SmallDesktopDisplaySetup(void);

void SmallDesktopDisplayLoop(void);

void SmallDesktopWeatherSet(struct SmallDesktopWeather weather);
