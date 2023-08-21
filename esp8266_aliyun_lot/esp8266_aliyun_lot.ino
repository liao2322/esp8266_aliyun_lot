
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>        //调用DHT库
 
/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "Liao的iPhone" 
#define WIFI_PASSWD       "18816010"

/*#define WIFI_SSID         "PDCN" 
#define WIFI_PASSWD       "1234567890"
*/
/* 设备的三元组信息*/
#define PRODUCT_KEY       "heiiCVLPRff" 
#define DEVICE_NAME       "ESP8266nodeMCU" 
#define DEVICE_SECRET     "5cccedb1fdd6820a3de60592080c5959"
#define REGION_ID         "cn-shanghai"
 
#define MQTT_SERVER        "heiiCVLPRff.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY
 
#define CLIENT_ID         "FESA234FBDS24|securemode=3,signmethod=hmacsha1,timestamp=789|" 
#define MQTT_PASSWD       "71630b4477c8313bd44942d680311a7e13720eeb"  

#define ALINK_BODY_FORMAT         "{\"id\":\"dht11\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
 
unsigned long lastMs = 0;
float RH,T,RH_sum,T_sum;
unsigned char count=0;//温湿度采集次数
WiFiClient espClient;
PubSubClient  client(espClient);

DHT dht(D1,DHT11);      //设置Data引脚所接IO口和传感器类型
 
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("消息发布到达 [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);
 
}
 /*WiFi初始化*/
void wifiInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);   //连接WiFi
    while (WiFi.status() != WL_CONNECTED)//持续不断请求接入网络
    {
        delay(1000);
        Serial.println("WiFi未连接");
    }
    Serial.println("连接到WiFi");
    Serial.println("IP地址为: ");//打印WiFi的本地IP
    Serial.println(WiFi.localIP());  
    Serial.print("espClient [");
    client.setServer(MQTT_SERVER, MQTT_PORT);   // 连接WiFi之后，连接MQTT服务器
    client.setCallback(callback);
}
 
 /*连接MQTT*/
void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("正在连接MQTT服务 ...");//MQTT正在连接
        if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
 
        {
            Serial.println("MQTT服务连接成功!");//MQTT连接成功
        }
        else
        {
            Serial.print("MQTT服务连接出错");//MQTT连接失败
            Serial.println(client.state());
            delay(5000);
        }
    }
}
 
void mqttIntervalPost()
{
    char param[32];    
    char jsonBuf[128];
    sprintf(param, "{\"CurrentTemperature\":%f}",T_sum/count);//发送温度平均值
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    //Serial.println(jsonBuf);//打印设备上传到云台的数据
    boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    if(d){
      Serial.println("发布温度数据成功！"); //上传温度成功
    }else{
      Serial.println("发布温度数据失败！"); //上传温度失败
    }     
    sprintf(param, "{\"CurrentHumidity\":%f}",RH_sum/count);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    //Serial.println(jsonBuf);
    d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    if(d){
      Serial.println("发布湿度数据成功！"); //上传湿度成功
    }else{
      Serial.println("发布湿度数据失败！"); //上传湿度失败
    }
}
 
void setup() 
{
    Serial.begin(115200);//波特率定义
    dht.begin();   
    Serial.println("温湿度检测开始");
    wifiInit();//WiFi初始化
    unsigned char i=0; 
}
 

void loop()
{
  delay(1000);                  //延时1000毫秒
    float RH = dht.readHumidity();   //读取湿度数据
    float T = dht.readTemperature();//读取温度数据
    RH_sum+=RH;
    T_sum+=T;
    count+=1;                
  Serial.print("湿度为:");  //向串口打印 Humidity:
  Serial.print(RH);           //向串口打印湿度数据
  Serial.print("%");
  Serial.print("  温度为:");//向串口打印 Temperature:
  Serial.print(T);            //向串口打印温度数据
  Serial.println("C"); 
  if (millis() - lastMs >= 1000)//数据上传时间间隔
  {
    lastMs = millis();//当前时间获取
    mqttCheckConnect(); //检查MQTT连接 
    mqttIntervalPost();
    count=0;
    RH_sum=0;
    T_sum=0;
  }
  client.loop();
}
