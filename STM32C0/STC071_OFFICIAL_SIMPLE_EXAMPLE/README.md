# STC071_OFFICIAL_SIMPLE_EXAMPLE

简洁说明：官方提供的 STM32 Nucleo 示例工程，针对 NUCLEO-C071RB，演示板级支持包（BSP）中 LED、按键以及虚拟串口（VCOM/USART）的基础使用。

## 支持硬件
- 开发板：NUCLEO-C071RB

## 主要功能
- 初始化并控制用户 LED（绿色、蓝色）
- 用户按键（PC13）以 EXTI 中断模式使用，按下时切换 LED 状态
- 虚拟串口（USART2，PA2/PA3）用于打印固件版本与欢迎信息（115200 8N1），BSP 提供 COM 接口支持

## 快速上手
1. 使用 IAR 打开工程：`EWARM/Project.eww`，加载工程后编译并使用 ST-Link 烧录运行。
2. 若使用其它工具链，请在工程中新建工程并添加 `Core/` 与 `Drivers/` 下的源文件与头文件。
3. 运行后，使用串口工具连接 Nucleo 虚拟串口（115200 8N1）查看输出。

## 目录与关键文件
- [STC071_OFFICIAL_SIMPLE_EXAMPLE.ioc](STC071_OFFICIAL_SIMPLE_EXAMPLE.ioc) — CubeMX 配置文件（若使用 CubeMX 生成代码）。
- [Core/Inc](Core/Inc) — 项目头文件目录（`main.h`、`gpio.h` 等）。
- [Core/Src](Core/Src) — 主程序与外设初始化源文件（`main.c`、`gpio.c` 等）。
	- 主程序: [Core/Src/main.c](Core/Src/main.c)
	- GPIO 初始化: [Core/Src/gpio.c](Core/Src/gpio.c)
- [Drivers/BSP/STM32C0xx_Nucleo](Drivers/BSP/STM32C0xx_Nucleo) — 板级支持包（LED、按键、COM）的实现与配置模板。
	- BSP 源: [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.c)
	- BSP 头: [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.h](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.h)
	- 配置模板: [Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf_template.h](Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf_template.h)
- [Drivers/STM32C0xx_HAL_Driver](Drivers/STM32C0xx_HAL_Driver) — HAL 驱动库源码（Inc/Src 文件夹）。
- [EWARM](EWARM) — IAR 项目文件与启动汇编、链接脚本等。

## 引脚速览（基于 BSP 定义）
- LED_GREEN: PA5
- LED_BLUE: PC9
- USER_BUTTON: PC13 (EXTI)
- VCOM (USART2): TX PA2, RX PA3

## 如何启用 VCOM（BSP COM）
1. 复制 `Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf_template.h` 为 `Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo_conf.h`。
2. 在该文件中：设置 `USE_BSP_COM_FEATURE` 为 `1`；根据板型选择 `USE_NUCLEO_64` 或 `USE_NUCLEO_48`。
3. 若使用不同时钟源，请调整 `Core/Src/main.c` 中的 `SystemClock_Config()` 或使用 CubeMX 重新生成时钟配置。

## 运行示例行为
- 上电/复位后，串口会打印软件版本及欢迎信息，两个 LED 将被点亮。
- 每次按下用户按键（PC13）会在中断回调中切换 LED 状态。

## 常见问题与排查提示
- 串口无输出：确认 `stm32c0xx_nucleo_conf.h` 中启用了 COM 功能，且开发板的 ST-Link VCP 驱动已安装；检查 `BSP_COM_Init` 返回值。
- LED 不亮：确认对应 GPIO 时钟已开启且引脚定义与板子匹配（参考 `Drivers/BSP/STM32C0xx_Nucleo/stm32c0xx_nucleo.h`）。
- 时钟配置问题：若没有外部 HSE，请在 `SystemClock_Config()` 中使用 HSI，或通过 CubeMX 配置并重新生成代码。

## 许可与参考
- HAL 与 BSP 源自 STMicroelectronics，参考 `Drivers/` 目录下的 LICENSE 文件与头文件注释。



