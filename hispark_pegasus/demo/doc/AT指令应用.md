# AT指令应用(连接WiFi)<a name="ZH-CN_TOPIC_0000001130176841"></a>

## 硬件环境搭建
-    硬件要求：Hi3861V100核心板、扩展板；硬件搭建及组网图如下图所示。
-    [Hi3861V100核心板参考：HiSpark_WiFi_IoT智能开发套件_原理图硬件资料\原理图\HiSpark_WiFi-IoT_Hi3861_CH340G_VER.B.pdf](http://gitee.com/hihope_iot/embedded-race-hisilicon-track-2022/blob/master/%E7%A1%AC%E4%BB%B6%E8%B5%84%E6%96%99/HiSpark_WiFi_IoT%E6%99%BA%E8%83%BD%E5%AE%B6%E5%B1%85%E5%BC%80%E5%8F%91%E5%A5%97%E4%BB%B6_%E5%8E%9F%E7%90%86%E5%9B%BE.rar)
-    [扩展板参考：HiSpark_WiFi_IoT智能开发套件_原理图硬件资料\原理图\HiSpark_WiFi-IoT_Hi3861_CH340G_VER.B.pdf](http://gitee.com/hihope_iot/embedded-race-hisilicon-track-2022/blob/master/%E7%A1%AC%E4%BB%B6%E8%B5%84%E6%96%99/HiSpark_WiFi_IoT%E6%99%BA%E8%83%BD%E5%AE%B6%E5%B1%85%E5%BC%80%E5%8F%91%E5%A5%97%E4%BB%B6_%E5%8E%9F%E7%90%86%E5%9B%BE.rar)

![输入图片说明](https://gitee.com/asd1122/tupian/raw/master/%E5%9B%BE%E7%89%87/%E5%9B%BE%E7%89%87200.png)

## 软件介绍

-   1.工程编译
    -   由于Hi3861V100源码自带AT指令,只需要编译源码就可以使用。工程相关配置完成后,然后编译。
-   2.烧录
    -   编译成功后，点击DevEco Home->配置工程->hi3861->upload_port->选择对应串口端口号->选择upload_protocol->选择hiburn-serial->点击save保存，在保存成功后点击upload进行烧录，出现connecting, please reset device..字样复位开发板等待烧录完成。
    -   烧录成功后，再次点击Hi3861核心板上的“RST”复位键，此时开发板的系统会运行起来。运行结果：打开串口工具并选择加回车换行，然后依次输入指令如下图所示，注意：当输入AT+CONN时候WiFi的认证方式有四种方式分别是0：OPEN，1：WEP，2：WPA2_PSK，3：WPA_WPA2_PSK，一般选择0或者2；可以实现AT指令连接wifi。
    ```
    AT+RST
    AT+STARTSTA
    AT+SCAN
    AT+SCANRESULT
    AT+CONN="WiFi名称",,WiFi认证方式,"WiFi密码"
    AT+DHCP=wlan0,1
    AT+IFCFG
    AT+PING=目的IP地址
    AT+DISCONN
    ```        
