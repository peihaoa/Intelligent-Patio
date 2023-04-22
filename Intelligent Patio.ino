#include <Arduino.h>
//I2C屏幕
#include <Wire.h>
#include <U8g2lib.h>

//数字温度传感器的库
#include <OneWire.h>
#include <DallasTemperature.h>

//空气温湿度传感器的库
#include "DHTesp.h"

//土壤湿度
int soil_Pin = 36; //模拟引脚A0连接传感器输入32
int soil_value = 0; //土壤湿度，越干燥读数越大

//OLED屏幕
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //esp32 23ms  SCL 22   SDA 21
char buf[50];

//数字温度
int digital_TMP=33;
OneWire oneWire(digital_TMP);
DallasTemperature sensor_TMP(&oneWire);
float temp_value=0;

//舵机
int freq = 50;      // 频率(20ms周期)
int channel = 8;    // 通道(高速通道（0 ~ 7）由80MHz时钟驱动，低速通道（8 ~ 15）由 1MHz 时钟驱动。)
int resolution = 8; // 分辨率
const int led = 32;//25
int deg=0;

//水泵
int channel_2=9;
const int led_2=17;

//光敏电阻
int ptr_Pin=39;//26
int ptr_value=0;

//CO2浓度
int co2_Pin=34;//14
int co2_value=0;

//DHT空气温湿度
int DHT_Pin = 16;
DHTesp dhtSensor;
float airTem=0.0;
float airHum=0.0;

//土壤湿度函数
int getSoil(){
  //Serial.println("GET_SOIL...");
  soil_value = analogRead(soil_Pin);
  return soil_value;
}

//数字温度函数
float getTmp(){
  sensor_TMP.requestTemperatures();
  temp_value=sensor_TMP.getTempCByIndex(0);
  return temp_value;
}

//舵机函数
void setDeg(int degree)
{ //0-180度
 //20ms周期，高电平0.5-2.5ms，对应0-180度角度
  const float deadZone = 6.4;//对应0.5ms（0.5ms/(20ms/256）)
  const float max = 32;//对应2.5ms
  if (degree < 0)
    degree = 0;
  if (degree > 180)
    degree = 180;
  int val=(int)(((max - deadZone) / 180) * degree + deadZone);
  ledcWrite(channel, val);
}

//光敏电阻函数
int getLight(){
  ptr_value=analogRead(ptr_Pin);
  return ptr_value;
}

//水泵函数
void pump(int val){
  ledcWrite(channel, val);
}

//CO2浓度函数
int getCO2(){
  co2_value=analogRead(co2_Pin);
  return co2_value;
}

//屏幕刷新打印函数
void oledPrint(float temperature,int light,int CO2,int degree ){
  u8g2.clearBuffer();
  u8g2.drawLine(0,15,127,15);
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.drawStr(0, 10, "Intelligent Patio");
  sprintf(buf,"Tem:%.2f Light:%d",temperature,light);
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 33, buf);
  sprintf(buf,"CO2:%d  degree:%d",CO2,degree);
  u8g2.drawStr(0, 46, buf);
  sprintf(buf,"airT:%.1f  airH:%.1f",airTem,airHum);
  u8g2.drawStr(0, 59, buf);
  u8g2.sendBuffer();

}

//空气温湿度函数
void DHT(){
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  airTem = data.temperature;
  airHum = data.humidity;
}

void setup() {
  Serial.begin(9600);
  //屏幕初始化
  u8g2.begin();

  //土壤湿度初始化
  pinMode(soil_Pin, INPUT);
  delay(2000);

  //数字温度初始化
  sensor_TMP.begin();

  //舵机初始化
  ledcSetup(channel, freq, resolution); // 设置通道
  ledcAttachPin(led, channel);          // 将通道与对应的引脚连接

  //水泵初始化
  ledcSetup(channel_2, freq, resolution); // 设置通道
  ledcAttachPin(led_2, channel_2);          // 将通道与对应的引脚连接

  //DHT22空气传感器初始化
  dhtSensor.setup(DHT_Pin, DHTesp::DHT22);

  Serial.println("HELLO Intelligent Patio!");

}

void loop() {
  DHT();
  pump(190);
  delay(2000);
  pump(0);
  Serial.printf("soil:%d temp:%.4f light:%d CO2:%d airTEM:%.2f airHUM:%.1f\n",getSoil(),getTmp(),getLight(),getCO2(),airTem,airHum);
  if(Serial.available()){
    deg=Serial.parseInt();
    if(deg==45){
      pump(190);
      delay(3000);
      pump(0);
    }
    Serial.parseInt();
    Serial.flush();
  }
  setDeg(deg);
  oledPrint(temp_value,ptr_value,co2_value,deg);
  delay(800);
}
