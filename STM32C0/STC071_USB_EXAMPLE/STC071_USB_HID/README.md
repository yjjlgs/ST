# STC071 USB HID 示例工程

简短说明：本工程为 NUCLEO-C071RB 的板级示例，演示板载 LED、用户按键、VCOM（虚拟串口）增加 USB HID（鼠标示例）功能，含 HAL 与 BSP 调用范例。

**支持硬件**

- 开发板：NUCLEO-C071RB（STM32C071）

**功能一览**

- 初始化并控制用户 LED（绿色、蓝色）
- 用户按键（PC13）作为 EXTI 中断输入，按键触发 LED 切换并移动鼠标箭头位置
- 虚拟串口（VCOM/USART）用于打印固件信息（115200 8N1）
- USB 设备（Device_Only），USBX + Device CoreStack FS，包含 HID（Mouse）演示
- USB 运行在 Full Speed（12MBit/s），使用内部 PHY

**快速上手**
1. 使用 IAR 打开工程文件：`EWARM/Project.eww`，或在你的 IDE 中导入 `Core/` 与 `Drivers/` 下的源文件。
2. 编译：在 IAR 中直接构建工程；若使用其他工具链，请确保包含 `Drivers/STM32C0xx_HAL_Driver` 与 `Drivers/BSP` 源文件。
3. 烧录并运行：使用 ST-Link 烧录板子，运行后通过虚拟串口查看输出（115200 8N1）。

**关键引脚速览**
- LED_GREEN: PA5
- LED_BLUE: PC9
- USER_BUTTON: PC13 (EXTI)
- VCOM (USART2): TX PA2, RX PA3
- USB(HID):USB_DM PA11 USB_DP PA12

**USB / USBX 配置**

- USB 模式：Device_Only
- 速率：Full Speed 12MBit/s
- 物理接口：Internal Phy

**主要文件与目录**

- [EWARM/Project.eww](EWARM/Project.eww) — IAR 工程文件
- [Core/Src/main.c](Core/Src/main.c) — 主程序入口与初始化
- [Core/Src/gpio.c](Core/Src/gpio.c) — LED 与按键 GPIO 初始化
- [Core/Inc/main.h](Core/Inc/main.h) — 全局定义
- [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c) — BSP 实现
- [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf_template.h](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf_template.h) — BSP 配置模板
- [Middlewares/ST/usbx](Middlewares/ST/usbx) 与 [USBX/App](USBX/App) — USBX 中间件与示例设备实现

**运行示例行为**

- 上电或复位后，固件会初始化时钟、HAL、BSP、USBX，并在串口打印固件版本与欢迎信息。
- 两个 LED 在初始化时点亮。
- 连接到主机后，USB HID（鼠标示例）将枚举为鼠标设备。
- 按下用户按键会触发中断回调切换 LED 状态并移动鼠标光标位置。

**常见问题与排查**
- 串口无输出：确认 `stm32c0xx_nucleo_conf.h` 中启用了 COM 功能，检查 ST-Link/VCP 驱动是否安装。
- LED 不亮：检查 `Core/Src/gpio.c` 中的引脚定义与板子实际引脚是否一致，并确认 GPIO 时钟已开启。
- USB 无法枚举：确认 USB 线缆、主机端驱动以及 USBX 内存分布配置是否正确；检查电源/接地。

**贡献与许可证**

- 若要提交改进或 issue，请在仓库中打开 PR 或 issue，并附上复现步骤与日志。
- HAL 与 BSP 源代码来自 STMicroelectronics，请参考 `Drivers/` 目录下的 LICENSE 文件获取具体许可信息。

---


