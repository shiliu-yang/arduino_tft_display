#include "Arduino.h"

#include "SmallDesktopDisplay.h"

#include "Ticker.h"

#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);

#include "TJpg_Decoder.h"

#include "number.h"
#include "weathernum.h"

Ticker AstTicker;

Number      dig;
WeatherNum  wrat;

/* *****************************************************************
 *  字库、图片库
 * *****************************************************************/
#include "font/ZdyLwFont_20.h"
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
      clkb.loadFont(ZdyLwFont_20);
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

unsigned char Hour_sign   = 60;
unsigned char Minute_sign = 60;
unsigned char Second_sign = 60;

unsigned char hour   = 12;
unsigned char minute = 34;
unsigned char second = 56;

String week = "周二";
String monthDay = "9月24日";

void digitalClockDisplay(int reflash_en)
{ 
  int timey=82;
  if(hour!=Hour_sign || reflash_en == 1)//时钟刷新
  {
    dig.printfW3660(20,timey,hour/10);
    dig.printfW3660(60,timey,hour%10);
    Hour_sign = hour;
  }
  if(minute!=Minute_sign  || reflash_en == 1)//分钟刷新
  {
    dig.printfO3660(101,timey,minute/10);
    dig.printfO3660(141,timey,minute%10);
    Minute_sign = minute;
  }
  if(second!=Second_sign  || reflash_en == 1)//分钟刷新
  {
    dig.printfW1830(182,timey+30,second/10);
    dig.printfW1830(202,timey+30,second%10);
    Second_sign = second;
  }
  
  if(reflash_en == 1) reflash_en = 0;
  /***日期****/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
   
  //星期
  clk.createSprite(58, 30);
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR);
  clk.drawString(week,29,16);
  clk.pushSprite(102,150);
  clk.deleteSprite();
  
  //月日
  clk.createSprite(95, 30);
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR);  
  clk.drawString(monthDay,49,16);
  clk.pushSprite(5,150);
  clk.deleteSprite();
  
  clk.unloadFont();
  /***日期****/
}

#if imgAst_EN
void imgAnim()
{
  int x=160,y=160;
  if(millis() - AprevTime > 37) //x ms切换一次
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

void SmallDesktopDisplaySetup(void)
{
  // tft.init();
  // tft.setRotation(0);
  // tft.fillScreen(0x0000);

  tft.begin(); /* TFT init */
  tft.invertDisplay(1);//反转所有显示颜色：1反转，0正常
  tft.setRotation(0);
  tft.fillScreen(0x0000);
  // tft.setTextColor(TFT_BLACK, BG_COLOR);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  /***绘制相关文字***/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);

  //温度
  TJpgDec.drawJpg(15,183,temperature, sizeof(temperature));  //温度图标

  // 温度 text
  clk.createSprite(58, 24);
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  clk.drawString("23℃",28,13);
  clk.pushSprite(100,184);
  clk.deleteSprite();

  //温度进度条
  tempWin(23,0x0AFF);

  //湿度
  TJpgDec.drawJpg(15,213,humidity, sizeof(humidity));  //湿度图标

  // 湿度 text
    clk.createSprite(58, 24); 
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  // clk.drawString(sk["SD"].as<String>(),28,13);
  clk.drawString("100%",28,13);
  clk.pushSprite(100,214);
  clk.deleteSprite();

  //湿度进度条
  humidityWin(100, 0x00FF);

  //城市名称
  clk.createSprite(94, 30); 
  clk.fillSprite(BG_COLOR);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, BG_COLOR); 
  clk.drawString("杭州",44,16);
  clk.pushSprite(15,15);
  clk.deleteSprite();

  //PM2.5空气指数
  uint16_t pm25BgColor = tft.color565(156,202,127);//优
  String aqiTxt = "优";
  clk.createSprite(56, 24); 
  clk.fillSprite(BG_COLOR);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0x0000); 
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(104,18);
  clk.deleteSprite();

  clk.unloadFont();

  //左上角滚动字幕
  scrollText[0] = "实时天气 晴";
  scrollText[1] = "空气质量 "+aqiTxt;
  scrollText[2] = "风向 西北风6级";
  scrollText[3] = "今日 晴";
  scrollText[4] = "最低温度 20 ℃";
  scrollText[5] = "最高温度 26 ℃";
  scrollBanner();

  //天气图标
  wrat.printfweather(170,15, 00);

  digitalClockDisplay(1);

// #if imgAst_EN
//   imgAnim();
//   AstTicker.attach(0.037, astCallback);
// #endif

  return;
}

void SmallDesktopDisplayLoop(void)
{
  imgAnim();
  return;
}
