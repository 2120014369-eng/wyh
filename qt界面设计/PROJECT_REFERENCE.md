# SerialDebugger 当前项目参考

> 本文档是 Qt 串口助手侧唯一保留的项目参考。旧的 32 位/release/dist、PA9/PA10 蓝牙、STM32 直连 K210、USART2 蓝牙说明都不要再作为依据。

## 项目位置

| 项目 | 路径 |
|------|------|
| Qt 上位机 | `D:\Qt\Doc\SerialDebugger` |
| STM32 固件 | `D:\MDK\DOC` |
| Qt 64 位 Debug 构建目录 | `D:\Qt\Doc\build-SerialDebugger-Desktop_Qt_5_14_2_MinGW_64_bit-Debug` |
| MDK 当前文档 | `D:\MDK\DOC\PROJECT_OVERVIEW.md` |

## 当前通信模型

- Qt 同时管理两个串口：
  - 小车蓝牙 COM：控制小车，`115200-8N1`。
  - K210 USB COM：触发拍照和接收 `IMG:*` 图片数据，`115200-8N1`。
- STM32 侧蓝牙固定接 `USART3 PB10/PB11`：
  - `PB10 TX` -> 蓝牙模块 `RXD`
  - `PB11 RX` <- 蓝牙模块 `TXD`
  - 共地
- `USART1 PA9/PA10` 预留给 STM32 系统串口 ISP 烧录，不接蓝牙。
- `PA3` 已确认损坏，不再使用 `PA2/PA3 USART2`。
- K210 不再接 STM32。K210 通过 USB 连接电脑，由 Qt 直接发送 `SNAP` 并接收图片。
- STM32 遇到障碍物后通过蓝牙回传 `OBS:<距离>mm`；Qt 识别 `OBS:` 后自动向 K210 USB 串口发送 `SNAP\r\n`。

## Qt 关键行为

方向按钮：

| 按钮 | 按下 | 松开 |
|------|------|------|
| 前进 | 周期发送单字节 `F` 到小车蓝牙串口 | 发送单字节 `S` |
| 后退 | 周期发送单字节 `B` 到小车蓝牙串口 | 发送单字节 `S` |
| 左转 | 周期发送单字节 `L` 到小车蓝牙串口 | 发送单字节 `S` |
| 右转 | 周期发送单字节 `R` 到小车蓝牙串口 | 发送单字节 `S` |
| 停止 | 点击发送 `S` 到小车蓝牙串口 | 无 |

普通点击按钮：

| 按钮 | 发送目标 | 发送内容 |
|------|----------|----------|
| 设置速度 | 小车蓝牙串口 | `V<n>\r\n` |
| 开启循迹 | 小车蓝牙串口 | `track\r\n` |
| 关闭循迹 | 小车蓝牙串口 | `track_off\r\n` |
| 继续前进 | 小车蓝牙串口 | `continue\r\n` |
| 保持停止 | 小车蓝牙串口 | `hold\r\n` |
| 查询状态 | 小车蓝牙串口 | `status\r\n` |
| 拍照 | K210 USB 串口 | `SNAP\r\n` |

默认设置：

- 默认波特率：`115200`
- 默认速度 UI：`40%`
- 方向长按发送间隔：`150ms`
- 手动文本发送默认追加 `\r\n`
- 方向单字节 `F/B/L/R/S` 是高频控制流，不启动接收超时；`track`、`status`、`V<n>` 等行命令才等待小车回复。

## 图片回传解析

K210 USB 串口返回：

- `K210:READY` / `K210:USB:READY`：脚本已启动。
- `K210:CAPTURE`：开始拍照。
- `IMG:BEGIN:<bytes>`：开始一张图。
- `IMG:DATA:<base64>`：追加 Base64 数据，不显示到接收区，也不写入普通日志，避免图片回传拖慢串口读取。
- `IMG:END`：Base64 解码，更新预览并保存图片。

保存目录：

```text
Documents/SerialDebuggerImages/yyyy-MM-dd/
```

日志文件：

```text
Documents/serial_debugger_log.txt
```

## 状态解析

Qt 当前会解析并更新诊断摘要：

- `MODE:`
- `SPEED:`
- `DIST:`
- `TRACK:`
- `OBS:`
- `BT_RX_OVF:`
- `K210:`
- `OK:`
- `Error:`

`BT:OK:*`、`BT:ERR:*`、`PC:*` 仅保留兼容解析，当前 STM32 固件不应再依赖这些旧前缀。

## 构建

只使用 64 位 Qt 5.14.2：

```powershell
cd 'D:\Qt\Doc\build-SerialDebugger-Desktop_Qt_5_14_2_MinGW_64_bit-Debug'
& 'D:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin\qmake.exe' 'D:\Qt\Doc\SerialDebugger\SerialDebugger.pro' -spec win32-g++ CONFIG+=debug CONFIG+=qml_debug
& 'D:\Qt\Qt5.14.2\Tools\mingw730_64\bin\mingw32-make.exe' -f Makefile.Debug
```

输出：

```text
D:\Qt\Doc\build-SerialDebugger-Desktop_Qt_5_14_2_MinGW_64_bit-Debug\debug\SerialDebugger.exe
```

不要再使用 32 位构建目录，也不需要 `dist`。

## STM32 对应事实

| 模块 | 当前配置 |
|------|----------|
| 蓝牙控制串口 | `USART3 PB10/PB11`, `115200` |
| ISP 烧录预留 | `USART1 PA9/PA10` |
| 禁用串口 | `USART2 PA2/PA3`，因为 `PA3` 损坏 |
| K210 | 电脑 USB，不接 STM32 |
| HC-SR04 | `PA11 TRIG`, `PA12 ECHO` |
| 循迹 | `PA4/PA5/PB1` |
| 默认速度 | `40%` |
| 固件输出 | `D:\MDK\DOC\Objects\neo.hex` |

STM32 编译：

```powershell
& 'D:\MDK\UV4\UV4.exe' -j0 -r 'D:\MDK\DOC\neo.uvprojx' -o 'D:\MDK\DOC\keil_build.log'
```

## 不要恢复的旧逻辑

- 不要把蓝牙放回 `PA9/PA10 USART1`，这组引脚保留给 ISP。
- 不要把蓝牙放回 `PA2/PA3 USART2`，`PA3` 已坏。
- 不要把 K210 接回 STM32 `PB10/PB11 USART3`；`PB10/PB11` 当前属于蓝牙。
- 不要把 K210 图片描述成经 STM32/蓝牙转发；当前图片走 K210 USB 串口。
- 不要把“蓝牙端口”做成独立于普通串口列表的重复区域。
- 不要用 `BT:OK:*` 或 `PC:*` 作为当前固件必须输出的协议。
- 不要用 32 位 Qt 构建产物继续排错。
