from pathlib import Path
from docx import Document
from docx.oxml import OxmlElement
from docx.text.paragraph import Paragraph

SOURCE = Path(r"C:\Users\jixiegeming\Downloads\（巡检机器人）9组-姓名-西南石油大学23级电子与计算机x班级x学号-报告(1).docx")
TARGET = SOURCE.with_name("（巡检机器人）9组-姓名-西南石油大学23级电子与计算机x班级x学号-报告(1)-按代码修订.docx")

def main():
    doc = Document(str(SOURCE))
    paras = list(doc.paragraphs)
    body_style = paras[87].style if len(paras) > 87 else doc.styles["Normal"]
    heading_style = paras[81].style if len(paras) > 81 else doc.styles["Heading 3"]

    replacements = {
        30: "本项目主要实现以下核心功能：第一，实现对电机的PWM调速与L298N方向控制；第二，实现基于USART3中断环形缓冲区的蓝牙串口通信；第三，完成多传感器（红外光电、超声波）的数据采集与逻辑判断；第四，设计完善的系统状态机以管理车辆运行模式；第五，实现STM32、K210与Qt上位机之间的多端协同。",
        34: "K210视觉模块与Qt上位机（PC端）通过USB CDC串口建立通信。当STM32小车遇到障碍物并通过蓝牙向Qt上位机发送“OBS:<距离>mm”警报时，上位机自动解析该信号，并向K210发送“SNAP”拍照指令。K210将JPEG图像转为Base64编码分块，通过“IMG:BEGIN / IMG:DATA / IMG:END”协议回传，最终由上位机完成解码、显示和保存。",
        41: "视觉模块K210使用MaixPy IDE进行交互式开发与调试；PC端上位机串口控制及图像显示软件使用Qt 5.14.2（基于MinGW 64-bit编译器）框架进行开发。上位机实际运行时同时打开蓝牙串口和K210 USB串口，分别完成运动控制与图像回传。",
        47: "K210视觉模块采用MicroPython编写boot.py脚本，完成RGB565/QVGA摄像头初始化、USB CDC命令解析、JPEG压缩以及Base64分包发送；上位机软件则采用C++面向对象编程，利用Qt框架的信号与槽机制处理蓝牙控制命令下发、状态解析以及K210图像数据的拼接渲染。",
        51: "主控采用STM32F103C8T6（Cortex-M3）芯片，运行频率72MHz。动力系统采用L298N双H桥驱动模块，其中PB8对应TIM4_CH3并连接右电机使能，PB9对应TIM4_CH4并连接左电机使能，PWM频率为1kHz；PB12、PB13控制右电机正反转方向，PB14、PB15控制左电机正反转方向。当前固件默认速度参数为40%。",
        53: "环境感知采用HC-SR04超声波测距模块（PA11为TRIG输出，PA12为ECHO输入，ECHO需降压到3.3V以内）和三路红外光电传感器（PA4、PA5、PB1）。通信部分采用HC-05/08蓝牙串口模块连接STM32的USART3（PB10/PB11），波特率115200，8位数据位、1位停止位、无校验；USART1（PA9/PA10）保留给系统ISP串口下载。视觉采集采用Kendryte K210核心板，通过USB数据线直接连接PC主机。",
        57: "STM32作为底层执行器，专注于实时性要求较高的电机运动、PWM输出、循迹状态读取和超声波测距，并通过蓝牙向外报告MODE、SPEED、DIST、TRACK、OBS、BT_RX_OVF等状态帧。Qt上位机作为指挥中心，同时打开两个串口：一个连接蓝牙控制小车，另一个连接K210 USB；当上位机收到小车的避障警报时，充当“桥梁”自动触发K210拍照。",
        59: "为了避免传统delay()延时导致的程序阻塞，STM32固件内部设计了基于SysTick（1ms）的状态机。系统定义了四种主要模式：CAR_MODE_MANUAL（手动）、CAR_MODE_TRACKING（循迹）、CAR_MODE_OBSTACLE_STOP（遇障停止）和CAR_MODE_WAIT_USER_DECISION（路口等待决策）。系统根据蓝牙指令、三黑线停靠条件以及超声波连续3次小于等于180mm、连续2次大于等于230mm等条件在不同状态间切换。",
        64: "主程序main.c承担了系统初始化、状态机流转以及指令解析的核心任务。程序启动后先执行约500000个空转周期以兼容串口ISP握手，随后初始化独立看门狗、GPIO、电机、超声波、循迹、USART3与SysTick，整体运行过程中不使用阻塞式延时代码。",
        66: "主循环中引入了system_ms全局毫秒时间戳。针对超声波测距任务，程序通过判断（system_ms - last_ultrasonic_check）>= 120来决定是否发起下一次测距，并以10mm为最小变化阈值回传DIST状态。在状态机流转方面，程序维护了obstacle_confirm_count和obstacle_clear_count变量，只有当超声波连续3次检测到小于等于180mm时才触发OBSTACLE_STOP，连续2次检测到大于等于230mm时才恢复循迹，从而抑制单次抖动。",
        68: "针对蓝牙控制，主程序实现了一套字符串与单字节混合的指令解析器。当my_usart_read_byte从环形缓冲区提取到\r或\n时，即认为一帧字符串指令接收完毕；而F/B/L/R/S也支持在cmd_len为0时直接作为单字节方向命令触发。当前固件已实现go、stop、track、track_off、continue、hold、status以及V0..V100等指令，并通过parse_speed_command、append_uint等自定义函数完成参数解析与状态拼接。",
        70: "3.2.1模块一：电机驱动模块 (mycar.c)",
        71: "该模块负责底层电机驱动。程序配置TIM4的CH3（PB8）和CH4（PB9）产生频率为1kHz的PWM波形控制两侧电机转速，并封装了go()、stop()、car_forward()、car_backward()、car_left()、car_right()和car_set_speed()等高层接口。方向控制通过PB12-PB15上的高低电平组合实现，支持前进、后退和原地差速转向。",
        73: "超声波模块采用轮询超时机制读取。程序先拉低TRIG 2us，再让PA11输出12us高电平触发测距，随后等待PA12回波输入变高。为防止传感器异常或回波丢失导致死循环，模块同时设置了30ms起始等待超时和25000us最大脉宽限制，任一条件触发时都立即返回0表示本次测距失败。",
        75: "循迹模块读取连接在PA4、PA5、PB1的红外光电传感器状态，当前代码将高电平定义为有效黑线信号。考虑到实物安装方向，程序启用了左右通道交换（TRACK_SWAP_LEFT_RIGHT=1），并通过3次采样消抖获得稳定模式值。控制策略未采用PID，而是使用基础速度的60%和25%进行轻微/强烈差速修正；当检测到111模式持续100ms时，系统进入WAIT_USER_DECISION，而000模式下则立即停车。",
        77: "该模块配置USART3工作在115200-8N1模式，PB10为TX、PB11为RX。核心设计是基于USART3_IRQHandler接收中断实现128字节环形缓冲区，并在中断中同时处理RXNE接收和ORE溢出清除。主程序通过头尾指针异步读取数据，保证了高频按键控制和状态回传同时进行时仍能稳定工作。",
        79: "视觉端采用K210芯片。K210上电执行boot.py脚本后，会初始化RGB565/QVGA摄像头并先后回传“K210:READY”和“K210:USB:READY:*”状态。Qt软件通过QSerialPort监听小车蓝牙串口，一旦检测到“OBS:”字符串，立刻向K210所在串口下发“SNAP\r\n”指令。K210拍照后按“IMG:BEGIN:<长度>”、“IMG:DATA:<Base64分块>”、“IMG:END”协议回传图像数据。",
        82: "系统上电后，上位机连接小车蓝牙串口。点击界面上的“前进”“左转”或设置速度值后，小车可立即响应并执行平稳运动；串口监视器能实时收到“OK: F”“SPEED:60”“OK: V60”等反馈。当电机处于运动状态时，板载PC13 LED按500ms节拍慢闪，停车时则保持常亮。",
        83: "发送 track 指令后，小车成功进入 MODE:TRACKING 状态。放置于铺设好的黑色胶带轨道上，小车能够自动根据轨迹左右微调方向，转弯平顺，全程未出现脱轨或乱跑现象。",
        85: "在循迹模式下，向小车正前方约18cm处放置障碍物（如纸箱）后，系统会在连续3次测得距离小于等于180mm后切换到MODE:OBSTACLE_STOP，并回传类似“OBS:175mm”的告警。小车会先暂停，待障碍物移开并连续2次检测到距离大于等于230mm后，系统自动恢复到MODE:TRACKING并继续循迹前行。",
        87: "小车因障碍物触发停车后，Qt上位机可成功捕获“OBS”关键字并调用triggerK210Snapshot()函数，通过K210 USB串口发送“SNAP”。随后上位机依次接收“K210:CAPTURE”“IMG:BEGIN”“IMG:DATA”与“IMG:END”等数据帧，并将Base64图像块拼合还原为现场障碍物照片。",
        89: "将前方障碍物移开并使测得距离连续2次达到23cm及以上后，小车会自动从MODE:OBSTACLE_STOP恢复到MODE:TRACKING，按120ms检测周期估算恢复延时约为240ms。当循迹行驶至“T”字型三岔路口或停止标识区，若三路传感器同时检测到黑线并持续约100ms，小车会进入WAIT_USER_DECISION状态；此时只有在上位机发送continue后才重新起步。",
        98: "通过本次基于STM32的智能巡检机器人项目实训，我将课堂上学到的微控制器原理与实际工程开发紧密结合。在底层硬件驱动方面，我熟练掌握了Keil MDK开发环境以及STM32标准外设库的使用，深入理解了定时器PWM、GPIO配置、USART3中断通信以及独立看门狗的工作机制。",
        99: "在项目推进过程中，我遇到了许多实际问题。例如，初期直接在主循环中读取串口会导致指令丢失；通过引入基于中断的128字节环形缓冲区，系统的通信稳定性得到明显提升。此外，以前习惯使用delay()函数，导致超声波测距时阻塞电机控制；后来我重构了代码，利用SysTick时间戳设计了一套非阻塞状态机，并将遇障停车、等待决策和自动恢复都纳入统一模式管理。最后，将K210机器视觉模块与C++ Qt上位机进行跨平台、跨语言的通信联调，也进一步锻炼了我的软硬件综合调试能力和工程化实现能力。",
    }

    for idx, text in replacements.items():
        paras[idx].text = text
        paras[idx].style = body_style if idx != 70 else heading_style

    paras[83].style = heading_style
    paras[84].style = body_style
    doc.save(str(TARGET))
    print(TARGET)

if __name__ == '__main__':
    main()
