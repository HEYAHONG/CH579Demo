# 说明

这是一个使用Keil5编写CH579程序的例子（仅供测试）。

## 目录说明

- doc: 一些文档，包括官方公开下载的datasheet。
- lib/CH579EVT:官方公开的库目录。
- lib/FreeRTOS:FreeRTOS源代码及配置文件。FreeRTOSConfig.h为FreeRTOS的配置文件，可根据自己需要定制各种选项。

# 编译条件

- 安装好keil5环境。
- 安装好设备支持包。[lib/CH579EVT/EVT/PUB/Keil.WCH57x_DFP.1.1.0.pack](./lib/CH579EVT/EVT/PUB/Keil.WCH57x_DFP.1.1.0.pack)

# 调试

可采用jlink直接调试，具体设置及问题参考lib/CH579EVT/中的例子。

# 其它开发方式说明

除了使用SDK进行开发,还可使用RT-Thread进行开发。

支持的BSP目录为：bsp/wch/arm/ch579m

## 相关链接

- https://github.com/RT-Thread/rt-thread.git
- https://gitee.com/rtthread/rt-thread.git
