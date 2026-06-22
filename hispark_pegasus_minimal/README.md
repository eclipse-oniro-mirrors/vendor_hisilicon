# 适配oh最小集
# 注意事项
- device/soc/hisilicon/hi3861v100/sdk_liteos/BUILD.gn修改
移除 "//base/security/device_auth/frameworks/deviceauth_lite:hichainsdk" 的固定依赖，改为条件依赖, "enable_oh_device_lite_auth=false" 时，不进行依赖。
# 编译命令
- 编译功能模块命令
hb set 选择 mini -> wifiiot_hispark_pegasus_minimal，然后hb bulid 

或者 `python3 build.py -p wifiiot_hispark_pegasus_minimal@hisilicon`
- 编译部分acts用例命令：
`python3 build.py -p wifiiot_hispark_pegasus_minimal@hisilicon --test xts //test/xts/acts/startup_lite/bootstrap_hal:ActsBootstrapTest,//test/xts/acts/distributed_schedule_lite/system_ability_manager_hal:ActsSamgrTest,//test/xts/acts/hiviewdfx_lite/hievent_hal:ActsHieventLiteTest,//test/xts/acts/startup_lite/syspara_hal:ActsParameterTest --gn-args build_xts=true --build-type=debug -f`