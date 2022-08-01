# HiSpark WiFi-IoT 套件样例开发--PWM（pwm_beer）

![hihope_illustration](https://gitee.com/hihopeorg/hispark-hm-pegasus/raw/master/docs/figures/hihope_illustration.png)

[HiSpark WiFi-IoT鸿蒙开发套件](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w5003-23341819265.1.bf644a82Da9PZK&id=622343426064&scene=taobao_shop) 首发于HDC 2020，是首批支持OpenHarmony 2.0的开发套件，亦是官方推荐套件，由润和软件HiHope量身打造，已在OpenHarmony社区和广大OpenHarmony开发者中得到广泛应用。

![wifi_iot](https://gitee.com/hihopeorg/hispark-hm-pegasus/raw/master/docs/figures/2.png)

## 一、PWM API

| API名称                                                      | 说明              |
| ------------------------------------------------------------ | ----------------- |
| unsigned int PwmInit(WifiIotPwmPort port);                   | PWM模块初始化     |
| unsigned int PwmStart(WifiIotPwmPort port, unsigned short duty, unsigned short freq); | 开始输出PWM信号   |
| unsigned int PwmStop(WifiIotPwmPort port);                   | 停止输出PWM信号   |
| unsigned int PwmDeinit(WifiIotPwmPort port);                 | 解除PWM模块初始化 |
| unsigned int PwmSetClock(WifiIotPwmClkSource clkSource);     | 设置PWM模块时钟源 |

## 二、交通灯板蜂鸣器按键与主控芯片（Pegasus）引脚的对应关系

- **蜂鸣器：**GPIO9/PWM0
- **按键：**GPIO8

## 三、如何编译

1. 将此目录下的 `pwm_beer_demo.c` 和 `BUILD.gn` 复制到openharmony源码的`applications\sample\wifi-iot\app\iothardware`目录下，
2. 修改openharmony源码的`applications\sample\wifi-iot\app\BUILD.gn`文件，将其中的 `features` 改为：

```
    features = [
        "iothardware:pwm_beer_demo",
    ]
```

3. 在openharmony源码顶层目录执行：`python build.py wifiiot`

## 四、运行结果

烧录文件后，按下reset按键，程序开始运行，按下按键后，蜂鸣器会响，再次按下按键，蜂鸣器不再发出响声



### 【套件支持】

##### 1. 套件介绍  http://www.hihope.org/pro/pro1.aspx?mtt=8

##### 2. 套件购买  https://item.taobao.com/item.htm?id=622343426064&scene=taobao_shop

##### 3. 技术资料

- Gitee码云网站（OpenHarmony Sample Code等) **https://gitee.com/hihopeorg**

- HiHope官网-资源中心（SDK包、技术文档下载）[**www.hihope.org**](http://www.hihope.org/)

##### 4. 互动交流

- 润和HiHope技术交流-微信群（加群管理员微信13605188699，发送文字#申请加入润和官方群#，予以邀请入群）
- HiHope开发者社区-论坛 **https://bbs.elecfans.com/group_1429**
- 润和HiHope售后服务群（QQ：980599547）
- 售后服务电话（025-52668590）

