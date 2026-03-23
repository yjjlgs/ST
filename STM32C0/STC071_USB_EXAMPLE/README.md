

# STC071 USB 示例工程集合

本仓库包含针对 STC071 系列 MCU 的多个 USB 示例工程，演示了不同 USB 设备类（CDC、HID、CDC+HID、Dual CDC）的使用方法与工程组织结构。每个子目录均为独立工程，并包含 IAR EWARM 工程文件、HAL 驱动、与中间件（USBX/USBX ST）示例。

## 目录
- [STC071_USB_CDC](STC071_USB_CDC/README.md) — USB CDC（虚拟串口）示例。
- [STC071_USB_HID](STC071_USB_HID/README.md) — USB HID（鼠标接口设备）示例。
- [STC071_USB_HID_CDC](STC071_USB_HID_CDC/README.md) — 同时演示 HID 与 CDC 的复合设备示例。
- [STC071_USB_HID_DualCDC](STC071_USB_HID_DualCDC/README.md) — HID + 双 CDC（两个虚拟串口）示例。

## 每个工程的主要内容（概览）
- STC071_USB_CDC: 演示如何将设备作为 USB CDC（虚拟串口）暴露给主机，包含 Core、Drivers、Middlewares（USBX）以及 IAR 工程文件。
- STC071_USB_HID: 演示 HID 报文的收发与处理，适用于自定义键盘/鼠标/控件类示例。
- STC071_USB_HID_CDC: 复合设备示例，同时实现 HID 与 CDC 接口，展示多接口描述符与组合通信逻辑。
- STC071_USB_HID_DualCDC: 在单设备上实现 HID 与两个 CDC 接口，适用于需要两个独立虚拟串口的应用场景。

## 快速开始
 - 开发工具链：IAR Embedded Workbench for ARM（工程文件位于各工程下的 `EWARM/` 目录）。
 - 配置：各工程包含 `.ioc`（CubeMX）文件用于外设配置（位于对应 `Core/` 或顶层目录），以及基于 STM32C0 HAL 的驱动代码。
 - 编译：打开对应子工程下的 IAR 工程文件（`.ewp`/`.eww`），选择目标后编译。
 - 烧录与调试：使用支持的 SWD 调试器或板载接口（按具体硬件）进行刷写与调试。

## 代码结构说明（通用）
- `Core/Inc` / `Core/Src`：应用入口、时钟与中断处理、USB 初始化相关代码。
- `Drivers/STM32C0xx_HAL_Driver`：HAL 驱动（移植自 ST 官方 HAL）。
- `Middlewares/ST` / `USBX`：USBX 或 ST 中间件示例代码，包含 USB 设备类实现。
- `EWARM/`：IAR 工程文件与链接脚本、启动文件。

## 如何贡献
- 若要改进或添加示例，请提交 issue 或 PR；请在提交前确保示例可构建并在 README 中说明硬件依赖与测试方法。

## 许可与致谢
各驱动与中间件可能包含各自的 LICENSE（见对应子目录），本仓库示例基于 ST 官方 HAL 与中间件改写或整合，感谢 ST 官方提供的资源。

