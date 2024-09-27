#include "Arduino.h"
#include "tal_time_service.h"

#include "SmallDesktopDisplay.h"

// #include "Ticker.h"

#include <SPI.h>
#include <TFT_eSPI.h>

#include "Log.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);

#include "TJpg_Decoder.h"

#include "number.h"
#include "weathernum.h"

// Ticker Ticker;

Number      dig;
WeatherNum  wrat;

struct SmallDesktopWeather weatherData, weatherDataPrev;

/* *****************************************************************
 *  字库、图片库
 * *****************************************************************/

// chinese font
// #include "font/ZdyLwFont_20.h"
#include "font/simhei20.h"
#define CHINESE_FONT simhei20


// #include "img/misaka.h"
#include "img/temperature.h"
#include "img/humidity.h"

#define imgAst_EN 1
#if imgAst_EN
#include "img/pangzi/i0.h"
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

int Anim = 0;           //太空人图标显示指针记录
int AprevTime = 0;      //太空人更新时间记录
#endif

#define BG_COLOR  TFT_BLACK

//TFT屏幕输出函数
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // PR_DEBUG("x: %d, y: %d, w: %d, h: %d", x, y, w, h);

  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return true;
}

// 温度进度条显示函数
void tempWin(int32_t tempnum, uint16_t tempcol)
{
  clk.setColorDepth(8);
  
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1,1,tempnum,4,2,tempcol);   //实心圆角矩形
  clk.pushSprite(45,192);  //窗口位置
  clk.deleteSprite();
}

// 湿度进度条显示函数
void humidityWin(int8_t huminum, uint16_t humicol)
{
  clk.setColorDepth(8);
  
  huminum = huminum/2;
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1,1,huminum,4,2,humicol);   //实心圆角矩形
  clk.pushSprite(45,222);  //窗口位置
  clk.deleteSprite();
}

String scrollText[7];
int currentIndex = 0;
TFT_eSprite clkb = TFT_eSprite(&tft);


void scrollBanner(){
  // if(millis() - prevTime > 2333) //3秒切换一次
//  if(second()%2 ==0&& prevTime == 0)
  // { 
    if(scrollText[currentIndex])
    {
      clkb.setColorDepth(8);
      clkb.loadFont(CHINESE_FONT);
      clkb.createSprite(150, 30); 
      clkb.fillSprite(BG_COLOR);
      clkb.setTextWrap(false);
      clkb.setTextDatum(CC_DATUM);
      clkb.setTextColor(TFT_WHITE, BG_COLOR); 
      clkb.drawString(scrollText[currentIndex],74, 16);
      clkb.pushSprite(10,45);
       
      clkb.deleteSprite();
      clkb.unloadFont();
      
      if(currentIndex>=5)
        currentIndex = 0;  //回第一个
      else
        currentIndex += 1;  //准备切换到下一个
    }
    // prevTime = 1;
  // }
}

uint8_t Hour = 0 , Minute = 0 , Second = 0;
uint8_t Month = 9 , Day = 27 , Week = 5;

void digitalClockDisplay(uint8_t month, uint8_t day, uint8_t week, uint8_t hour, uint8_t minute, uint8_t second)
{
  static uint8_t lastHour = -1, lastMinute = -1, lastSecond = -1;
  static uint8_t lastMonth = -1, lastDay = -1, lastWeek = -1;

  int timey=82;

  if (lastHour != hour) {
    dig.printfW3660(20,timey,hour/10);
    dig.printfW3660(60,timey,hour%10);
    lastHour = hour;
  }

  if (lastMinute != minute) {
    dig.printfO3660(101,timey,minute/10);
    dig.printfO3660(141,timey,minute%10);
    lastMinute = minute;
  }

  if (lastSecond != second) {
    dig.printfW1830(182,timey+30,second/10);
    dig.printfW1830(202,timey+30,second%10);
    lastSecond = second;
  }

  const char *weekStr[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
  if (lastWeek != week || lastMonth != month || lastDay != day) {
    clk.setColorDepth(8);
    clk.loadFont(CHINESE_FONT);

    //星期
    clk.createSprite(58, 30);
    clk.fillSprite(BG_COLOR);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, BG_COLOR);
    clk.drawString(weekStr[week],29,16);
    clk.pushSprite(102,150);
    clk.deleteSprite();
    lastWeek = week;

    // 月日
    clk.createSprite(95, 30);
    clk.fillSprite(BG_COLOR);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, BG_COLOR);
    clk.drawString(String(month) + "月" + String(day) + "日",49,16);
    clk.pushSprite(5,150);
    clk.deleteSprite();
    lastMonth = month;
    lastDay = day;

    clk.unloadFont();
  }
}

// void digitalClockDisplay(int reflash_en)
// { 
//   int timey=82;
//   if(hour!=Hour_sign || reflash_en == 1)//时钟刷新
//   {
//     dig.printfW3660(20,timey,hour/10);
//     dig.printfW3660(60,timey,hour%10);
//     Hour_sign = hour;
//   }
//   if(minute!=Minute_sign  || reflash_en == 1)//分钟刷新
//   {
//     dig.printfO3660(101,timey,minute/10);
//     dig.printfO3660(141,timey,minute%10);
//     Minute_sign = minute;
//   }
//   if(second!=Second_sign  || reflash_en == 1)//分钟刷新
//   {
//     dig.printfW1830(182,timey+30,second/10);
//     dig.printfW1830(202,timey+30,second%10);
//     Second_sign = second;
//   }
  
//   if(reflash_en == 1) reflash_en = 0;
//   /***日期****/
//   clk.setColorDepth(8);
//   clk.loadFont(CHINESE_FONT);

//   //星期
//   clk.createSprite(58, 30);
//   clk.fillSprite(BG_COLOR);
//   clk.setTextDatum(CC_DATUM);
//   clk.setTextColor(TFT_WHITE, BG_COLOR);
//   clk.drawString(week,29,16);
//   clk.pushSprite(102,150);
//   clk.deleteSprite();
  
//   //月日
//   clk.createSprite(95, 30);
//   clk.fillSprite(BG_COLOR);
//   clk.setTextDatum(CC_DATUM);
//   clk.setTextColor(TFT_WHITE, BG_COLOR);  
//   clk.drawString(monthDay,49,16);
//   clk.pushSprite(5,150);
//   clk.deleteSprite();
  
//   clk.unloadFont();
//   /***日期****/
// }

#if imgAst_EN
void imgAnim()
{
  int x=160,y=160;
  if(millis() - AprevTime > 27) //x ms切换一次
  {
    Anim++;
    AprevTime = millis();
  }
  if(Anim==10)
    Anim=0;

  switch(Anim)
  {
    case 0:
      TJpgDec.drawJpg(x,y,i0, sizeof(i0));
      break;
    case 1:
      TJpgDec.drawJpg(x,y,i1, sizeof(i1));
      break;
    case 2:
      TJpgDec.drawJpg(x,y,i2, sizeof(i2));
      break;
    case 3:
      TJpgDec.drawJpg(x,y,i3, sizeof(i3));
      break;
    case 4:
      TJpgDec.drawJpg(x,y,i4, sizeof(i4));
      break;
    case 5:
      TJpgDec.drawJpg(x,y,i5, sizeof(i5));
      break;
    case 6:
      TJpgDec.drawJpg(x,y,i6, sizeof(i6));
      break;
    case 7:
      TJpgDec.drawJpg(x,y,i7, sizeof(i7));
      break;
    case 8: 
      TJpgDec.drawJpg(x,y,i8, sizeof(i8));
      break;
    case 9: 
      TJpgDec.drawJpg(x,y,i9, sizeof(i9));
      break;
    default:
      Serial.println("显示Anim错误");
      break;
  }
}
#endif

void astCallback() {
  // PR_NOTICE("Free heap: %d", tal_system_get_free_heap_size());
  imgAnim();
}

void TemperatureSet(int temperature)
{
  uint16_t tempcol = 0;

  clk.setColorDepth(8);
  clk.loadFont(CHINESE_FONT);

  clk.createSprite(58, 24);
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  clk.drawString(String(temperature) + "℃",28,13);
  clk.pushSprite(100,184);
  clk.deleteSprite();

  temperature = temperature+10;
  if(temperature<10)
    tempcol=0x00FF;
  else if(temperature<28)
    tempcol=0x0AFF;
  else if(temperature<34)
    tempcol=0x0F0F;
  else if(temperature<41)
    tempcol=0xFF0F;
  else if(temperature<49)
    tempcol=0xF00F;
  else
  {
    tempcol=0xF00F;
    temperature=50;
  }

  // 温度进度条
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1, 1, temperature, 4, 2, tempcol);   //实心圆角矩形
  clk.pushSprite(45,192);  //窗口位置
  clk.deleteSprite();

  clk.unloadFont();

  return;
}

void CitySet(String cityName)
{
  clk.setColorDepth(8);
  clk.loadFont(CHINESE_FONT);

  clk.createSprite(94, 30); 
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  clk.drawString(cityName,44,16);
  clk.pushSprite(15,15);
  clk.deleteSprite();

  clk.unloadFont();

  return;
}

void HumiditySet(int humidity)
{
  uint16_t humicol = 0;

  clk.setColorDepth(8);
  clk.loadFont(CHINESE_FONT);

  clk.createSprite(58, 24); 
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  clk.drawString(String(humidity) + "%",28,13);
  clk.pushSprite(100,214);
  clk.deleteSprite();

  if(humidity>90)
    humicol=0x00FF;
  else if(humidity>70)
    humicol=0x0AFF;
  else if(humidity>40)
    humicol=0x0F0F;
  else if(humidity>20)
    humicol=0xFF0F;
  else
    humicol=0xF00F;

  //湿度进度条
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1,1, humidity/2 ,4,2,humicol);   //实心圆角矩形
  clk.pushSprite(45,222);  //窗口位置
  clk.deleteSprite();

  clk.unloadFont();

  return;
}

void aqiSet(int aqi)
{
  uint16_t pm25BgColor = tft.color565(156,202,127);//优
  String aqiTxt = "优";

  clk.setColorDepth(8);
  clk.loadFont(CHINESE_FONT);

  clk.createSprite(56, 24); 
  clk.fillSprite(BG_COLOR);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0x0000); 
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(104,18);
  clk.deleteSprite();

  clk.unloadFont();
}

void SmallDesktopDisplaySetup(void)
{
  tft.begin(); /* TFT init */
  tft.invertDisplay(1);//反转所有显示颜色：1反转，0正常
  tft.setRotation(0);
  tft.fillScreen(0x0000);

  weatherDataPrev.weatherCode = 0;
  weatherDataPrev.realFeel = 0;
  weatherDataPrev.humidity = 0;
  weatherDataPrev.aqi = 0;
  weatherDataPrev.highTemp = 0;
  weatherDataPrev.lowTemp = 0;
  weatherDataPrev.cityName = "";
  weatherDataPrev.windDir = "";
  weatherDataPrev.windLevel = 0;

  weatherData.weatherCode = 120;
  weatherData.realFeel = 26;
  weatherData.humidity = 70;
  weatherData.aqi = 34;
  weatherData.highTemp = 30;
  weatherData.lowTemp = 22;
  weatherData.cityName = "杭州";
  weatherData.windDir = "NE";
  weatherData.windLevel = 2;

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  //温度
  TJpgDec.drawJpg(15,183,temperature, sizeof(temperature));  //温度图标
  //湿度
  TJpgDec.drawJpg(15,213,humidity, sizeof(humidity));  //湿度图标

  //左上角滚动字幕
  scrollText[0] = "实时天气 晴";
  scrollText[1] = "空气质量 优";
  scrollText[2] = "风向 西北风6级";
  scrollText[3] = "今日 晴";
  scrollText[4] = "最低温度 20 ℃";
  scrollText[5] = "最高温度 26 ℃";
  scrollBanner();

  //天气图标
  wrat.printfweather(170,15, 00);

  return;
}

void SmallDesktopDisplayLoop(void)
{
  int rt = OPRT_OK;
  POSIX_TM_S posixTime;
  rt = tal_time_get_local_time_custom(0, &posixTime);
  if (rt == OPRT_OK) {
    Month = posixTime.tm_mon + 1;
    Day = posixTime.tm_mday;
    Week = posixTime.tm_wday;
    Hour = posixTime.tm_hour;
    Minute = posixTime.tm_min;
    Second = posixTime.tm_sec;
    digitalClockDisplay(Month, Day, Week, Hour, Minute, Second);
  }

  // if (weatherData.weatherCode != weatherDataPrev.weatherCode) {
  //   wrat.printfweather(170,15, weatherData.weatherCode);
  //   weatherDataPrev.weatherCode = weatherData.weatherCode;
  // }

  if (weatherData.realFeel != weatherDataPrev.realFeel) {
    TemperatureSet(weatherData.realFeel);
    weatherDataPrev.realFeel = weatherData.realFeel;
  }

  if (weatherData.humidity != weatherDataPrev.humidity) {
    HumiditySet(weatherData.humidity);
    weatherDataPrev.humidity = weatherData.humidity;
  }

  if (weatherData.cityName != weatherDataPrev.cityName) {
    CitySet(weatherData.cityName);
    weatherDataPrev.cityName = weatherData.cityName;
  }

  if (weatherData.aqi != weatherDataPrev.aqi) {
    aqiSet(weatherData.aqi);
    weatherDataPrev.aqi = weatherData.aqi;
  }

  imgAnim();

  return;
}

void SmallDesktopWeatherSet(struct SmallDesktopWeather weather)
{
  weatherData.weatherCode = weather.weatherCode;
  weatherData.realFeel = weather.realFeel;
  weatherData.humidity = weather.humidity;
  weatherData.aqi = weather.aqi;
  weatherData.highTemp = weather.highTemp;
  weatherData.lowTemp = weather.lowTemp;
  weatherData.cityName = weather.cityName;
  weatherData.windDir = weather.windDir;
  weatherData.windLevel = weather.windLevel;

  return;
}
