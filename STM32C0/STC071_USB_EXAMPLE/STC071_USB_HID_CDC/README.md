
# STC071 USB HID + CDC 示例工程

简短说明：本工程针对 NUCLEO-C071RB（STM32C071），演示板载 LED、用户按键、以及 USB 设备类同时包含 CDC ACM（虚拟串口）与 HID 鼠标的组合使用。工程基于 HAL + BSP 与 USBX 中间件，演示 CDC 与 HID 的注册、数据收发以及与板级外设（UART、LED、按键）的交互。

**支持硬件**

- 开发板：NUCLEO-C071RB（STM32C071）

**功能一览**

- 初始化并控制用户 LED（绿色、蓝色）
- 用户按键（PC13）作为 EXTI 中断输入，按键触发 LED 切换、触发 HID 鼠标事件以及触发 CDC 向主机发送预定义数据。
- USB 设备（Device_Only），使用 USBX Device CoreStack（Full Speed）同时注册 CDC ACM 与 HID Mouse 两个类。
- CDC 功能：主机发送的数据由固件接收后转发到板载串口 COM1（USART2）；固件在按键触发时发送 `UserTxBufferFS` 数据到主机。实现位于 `USBX/App/ux_device_cdc_acm.c`。
- HID 功能：按键触发产生鼠标相对移动事件（模拟移动一个方形轨迹），实现位于 `USBX/App/ux_device_mouse.c`。

**快速上手**
1. 使用 IAR 打开工程：EWARM/Project.eww，或在其它 IDE 中导入 `Core/`、`Drivers/` 、 `USBX/App` 下的源文件进行移植。
2. 编译：在 IAR 中直接构建；若使用其他工具链，请确保包含 `Drivers/STM32C0xx_HAL_Driver`、`Drivers/BSP`、以及 USBX 中间件的源文件和头文件。
3. 烧录并运行：使用 ST-Link 烧录开发板，运行后主机端会枚举出两个接口：CDC 虚拟串口（VCOM），以及 HID 鼠标。打开串口终端查看串口输出，同时主机会看到鼠标设备（可观察到按键触发的光标移动）。

**关键引脚速览**
- LED_GREEN: PA5
- LED_BLUE: PC9
- USER_BUTTON: PC13 (EXTI)
- 硬件串口（USART2）：TX PA2, RX PA3
- USB: USB_DM PA11, USB_DP PA12

**USB / USBX 配置**

- USB 模式：Device_Only
- 速率：Full Speed（12 Mbit/s）
- 物理接口：Internal PHY
- 中间件：使用 ST 的 USBX（见 `Middlewares/ST/usbx`），设备端应用代码位于 `USBX/App`。`app_usbx_device.c` 在启动时注册 HID 与 CDC 两个类并初始化 USB 设备栈。

**主要文件与目录**

- [EWARM/Project.eww](EWARM/Project.eww) — IAR 工程文件
- [Core/Src/main.c](Core/Src/main.c) — 主程序入口、USBX 初始化与主循环（调用 `USBX_Device_Process`）
- [Core/Src/gpio.c](Core/Src/gpio.c) — GPIO 初始化（工程使用 BSP 处理板级引脚）
- [Core/Inc/main.h](Core/Inc/main.h) — 全局定义
- [Core/Inc/usb.h](Core/Inc/usb.h) 与 [Core/Src/usb.c](Core/Src/usb.c) — USB 初始化与回调（PCD 层）
- [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c) — 板級支持包（LED、按键、COM1）
- [USBX/App/app_usbx_device.c](USBX/App/app_usbx_device.c) — 在 USBX 中注册 HID/CDC 类并初始化设备
- [USBX/App/ux_device_cdc_acm.c](USBX/App/ux_device_cdc_acm.c) — CDC 实现（读/写任务、向串口转发）
- [USBX/App/ux_device_mouse.c](USBX/App/ux_device_mouse.c) — HID 鼠标实现（按键触发移动事件）

**运行示例行为**

- 上电或复位后，固件初始化时钟、HAL、BSP 与 USBX；启动信息（固件版本、欢迎信息）通过板载串口 COM1（USART2）输出，见 `Core/Src/main.c`。
- 两个 LED 在初始化后短暂点亮以指示启动完成。
- 将开发板连接到主机后，主机会枚举出 CDC 虚拟串口设备（可在设备管理器或 /dev 下看到）以及 HID 鼠标设备；分别用于串口数据交互与鼠标事件。
- 按下用户按键：
	- 切换 LED 状态（`BSP_LED_Toggle`）；
	- 将 `tx_pending` 标志置位，触发 `CDC_ACM_Write_Task` 发送 `UserTxBufferFS`（预填充的示例数据）到主机；
	- 将 `mouse_state` 标志置位，触发 `USBX_DEVICE_HID_MOUSE_Task` 发送一次鼠标移动事件到主机（实现为方形轨迹的相对移动）。
- 主机向 CDC 端发送数据时，固件在 `CDC_ACM_Read_*_Task` 中接收数据并通过 COM1（USART2）向外输出（UART 转发）。
- 工程内启用了简单的 CDC 速率统计（宏 `ENABLE_CDC_SPEED_TEST`），在串口上打印接收/发送帧的字节数与速率统计（可在 `USBX/App/ux_device_cdc_acm.c` 中关闭）。

**常见问题与排查**

- CDC/鼠标不可见：确认 ST-Link/USB 驱动已安装；尝试更换 USB 线缆或主机端口；在设备管理器/lsusb 中查看是否有枚举结果。
- 串口无输出：确认 `USE_BSP_COM_FEATURE` 已在 BSP 配置中启用，且 `BSP_COM_Init` 成功返回；检查 `Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c` 中 COM 引脚定义是否与实际板子一致。
- HID 无动作：确保主机已识别 HID 设备，且在按键按下时固件将 `mouse_state` 置位；检查 `USBX_Device_Process` 是否在主循环中被调用（见 `Core/Src/main.c`）。
- USB 无法枚举：检查 USB 引脚（PA11/PA12）、时钟（HSI48/HSE）与 USBX 内存池大小，确认 `MX_USB_PCD_Init()` 成功且 `HAL_PCD_Start()` 被调用。

**贡献与许可证**

- 提交改进或 issue：请在仓库中打开 PR 或 issue，并附上复现步骤与日志。
- HAL、BSP 与 USBX 源代码来自 STMicroelectronics，请参考 `Drivers/` 与 `Middlewares/` 目录下的 LICENSE 文件获取具体许可信息。

---


