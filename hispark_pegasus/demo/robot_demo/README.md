# HiSpark-Pegasus 智能小车样例

## 智能小车套件外设控制样例

### 文件说明

| 文件             | 说明           |
| ---------------- | -------------- |
| robot_hcsr04.c   | 超声波测距样例 |
| robot_l9110s.c   | 电机驱动样例   |
| robot_sg90.c     | 舵机控制样例   |
| robot_sg90_mid.c | 舵机校准       |
| robot_tcrt5000.c | 红外传感器样例 |

### 使用方法

1、直接将robot文件夹拷贝到OpenHarmony源码applications/sample/wifi-iot/app目录下。

2、需要编译哪个样例，就将对应的文件的注释打开，将其他的文件注释掉

​      比如需要编译超声波样例，BUILD.gn文件修改为如下内容：

```
static_library("robot_demo") {
    sources = [
        "robot_hcsr04.c",
        #"robot_l9110s.c",
        #"robot_sg90_mid.c",
        #"robot_sg90.c",
        #"robot_tcrt5000.c"
        #"l9110s_pwm_control.c"
    ]

    include_dirs = [
        "//utils/native/lite/include",
        "//kernel/liteos_m/kal/cmsis",
        "//base/iothardware/peripheral/interfaces/inner_api",
    ]
}
```

3、修改app目录下的BUILD.gn文件

```
import("//build/lite/config/component/lite_component.gni")

lite_component("app") {
    features = [
        "robot:robot_demo"
    ]
}
```

## 智能循迹避障小车样例代码

### 文件说明

| 文件            | 说明                   |
| --------------- | ---------------------- |
| ssd1306         | ssd1306驱动代码        |
| robot_control.c | 智能小车主要控制代码   |
| robot_hcsr04.c  | 智能小车超声波测距代码 |
| robot_l9110s.c  | 智能小车电机控制代码   |
| robot_sg90.c    | 智能小车舵机控制代码   |
| ssd1306_test.c  | 智能小车OLED显示屏代码 |
| trace_model.c   | 智能小车循迹功能代码   |

### 使用方法

1、直接将robot_demo文件夹拷贝到OpenHarmony源码applications/sample/wifi-iot/app目录下。

2、需要编译哪个样例，就将对应的文件的注释打开，将其他的文件注释掉

​      比如需要编译超声波样例，BUILD.gn文件修改为如下内容：

```
static_library("robot_demo") {
    sources = [
        "robot_hcsr04.c",
        "robot_l9110s.c",
        "robot_sg90.c",
        "trace_model.c",
        "ssd1306_test.c",
        "robot_control.c"
    ]

    include_dirs = [
        "./ssd1306",
        "//utils/native/lite/include",
        "//kernel/liteos_m/kal/cmsis",
        "//base/iothardware/peripheral/interfaces/inner_api",
    ]
}
```

3、修改app目录下的BUILD.gn文件

```
import("//build/lite/config/component/lite_component.gni")

lite_component("app") {
    features = [
        "robot_demo:robot_demo"
    ]
}
```

