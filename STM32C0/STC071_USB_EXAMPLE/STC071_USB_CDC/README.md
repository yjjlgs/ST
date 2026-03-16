
# STC071 USB CDC (VCOM) 示例工程

简短说明：本工程为 NUCLEO-C071RB（STM32C071）的板级示例，演示板载 LED、用户按键与 USB 虚拟串口（CDC ACM / VCOM）功能，包含 HAL 与 BSP 的调用示例，便于在主机上通过虚拟串口交互。

**支持硬件**

- 开发板：NUCLEO-C071RB（STM32C071）

**功能一览**

- 初始化并控制用户 LED（绿色、蓝色）
-- 用户按键（PC13）作为 EXTI 中断输入，按键触发 LED 状态切换并通过 USB CDC（虚拟串口）发送事件信息至主机
- USB 设备（Device_Only），使用 USBX + Device CoreStack FS，实现 CDC ACM（虚拟串口）功能
- USB 运行在 Full Speed（12 Mbit/s），使用内部 PHY

**快速上手**
1. 使用 IAR 打开工程：EWARM/Project.eww，或在你的 IDE 中导入 `Core/` 与 `Drivers/` 下的源文件进行移植。
2. 编译：在 IAR 中直接构建工程；若使用其他工具链，请确保包含 `Drivers/STM32C0xx_HAL_Driver` 与 `Drivers/BSP` 的源文件和头文件。
3. 烧录并运行：使用 ST-Link 烧录开发板，运行后主机端会识别一个 USB 虚拟串口设备（Windows 下为 COM 端口）。打开串口终端（波特率由主机设置，但 CDC 本身不固定波特率），查看固件输出。

**关键引脚速览**
- LED_GREEN: PA5
- LED_BLUE: PC9
- USER_BUTTON: PC13 (EXTI)
- 硬件串口（USART2）：TX PA2, RX PA3
- USB: USB_DM PA11, USB_DP PA12

**USB / USBX 配置**

- USB 模式：Device_Only
- 速率：Full Speed 12 Mbit/s
- 物理接口：Internal Phy
- 中间件：使用 ST 的 USBX 实现 CDC ACM 设备类，相关配置位于 `Middlewares/ST/usbx` 与 `USBX/App` 下。

**主要文件与目录**

- [EWARM/Project.eww](EWARM/Project.eww) — IAR 工程文件
- [Core/Src/main.c](Core/Src/main.c) — 主程序入口与系统初始化
- [Core/Src/gpio.c](Core/Src/gpio.c) — LED 与按键 GPIO 初始化与控制
- [Core/Inc/main.h](Core/Inc/main.h) — 全局定义
- [Core/Inc/usb.h](Core/Inc/usb.h) 与 [Core/Src/usb.c](Core/Src/usb.c) — USB 初始化与回调
- [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c) — 板级支持包（BSP）实现
- [Middlewares/ST/usbx](Middlewares/ST/usbx) 与 [USBX/App](USBX/App) — USBX 中间件与 CDC 设备示例实现（如 `app_usbx_device.c`）

**运行示例行为**


- 上电或复位后，固件初始化时钟、HAL、BSP 与 USBX；默认情况下，启动信息（例如固件版本与欢迎信息）通过板载硬件串口 COM1（USART2）输出。
- 两个 LED 在初始化后短暂点亮以指示启动完成。
- 将开发板连接到主机后，主机会枚举出一个 CDC 虚拟串口设备（在设备管理器或 /dev 下可见）。可使用串口终端与设备交互。
- 按下用户按键会触发中断回调：切换 LED 状态并通过 USB CDC（虚拟串口）发送一条事件消息到主机（当前实现发送预定义的 `UserTxBufferFS` 数据）。
- 主机（PC）可以向设备发送数据；当前固件实现会将接收到的数据转发到板载硬件串口 COM1（USART2）。

**常见问题与排查**

- 虚拟串口不可见：确认 ST-Link/VCP 驱动已安装；在设备管理器中检查是否出现新的 COM 端口；尝试更换 USB 线缆或主机端 USB 端口。
- 串口无输出：检查 `stm32c0xx_nucleo_conf.h` 中是否启用了 VCOM/CDC 支持；确认 `app_usbx_device.c`（或等效文件）已正确初始化 CDC 并启动 USBX。
- LED 不亮：检查 `Core/Src/gpio.c` 中的引脚定义与实际板子一致，并确认 GPIO 时钟已开启。
- USB 无法枚举：检查 USB 接线、主机驱动与 USBX 内存分配配置（栈大小、缓存位置等）；确认电源与接地良好。

**贡献与许可证**

- 提交改进或 issue：请在仓库中打开 PR 或 issue，并附上复现步骤与日志。
- HAL 与 BSP 源代码来自 STMicroelectronics，请参考 `Drivers/` 目录下的 LICENSE 文件获取具体许可信息。

---


