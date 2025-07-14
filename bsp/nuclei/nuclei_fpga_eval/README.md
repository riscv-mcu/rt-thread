# 芯来科技FPGA评估板

## 简介

**芯来科技FPGA系列板** 是由芯来科技公司推出的用于一系列测试评估芯来RISC-V内核处理器的FPGA评估板。

更多关于 **芯来科技FPGA评估板** 开发板的详细资料请参见:

* [Nuclei DDR200T开发板](https://nucleisys.com/developboard.php#ddr200t)
* [Nuclei KU060开发板](https://nucleisys.com/developboard.php#hp060)

### 板载资源

| 硬件 | 描述 |
| ---  | --- |
| 内核 | Nuclei RISC-V N/NX/UX 内核(200/300/600/900/1000 series) |
| 架构 | RV32 or RV64 |
| 主频 | 16MHz or 50MHz or uncertain freq |

**注意**: 这个上面烧写的是FPGA bitstream文件，所以处理器内核版本根据型号来定，通过修改**rtconfig.py**中的**NUCLEI_SDK_CORE**.

## 工具安装

在如下环境下验证可以正常工作:

- [**Nuclei Studio 2025.02**](https://nucleisys.com/download.php)
- [**Nuclei SDK >= 0.8.1**](https://github.com/Nuclei-Software/nuclei-sdk/tree/d24c4e9242ca8f7a4b85fc91edb08e555b97aedc)
- [**RT-Thread env-windows-v2.0.0**](https://www.rt-thread.org/document/site/#/development-tools/env/env)

### 安装工具链

请根据[安装Nuclei RISC-V GCC Toolchain和OpenOCD](https://doc.nucleisys.com/nuclei_sdk/quickstart.html#setup-tools-and-environment) 来安装依赖的工具。

> - 支持 Nuclei Studio >= 2025.02, Toolchain PREFIX=`riscv64-unknown-elf-`


### 添加环境变量

将Nuclei RISC-V GCC Toolchain和OpenOCD的环境变量进行设置。

#### Windows

假设工具安装在 **D:\NucleiStudio\toolchain**目录下, 则可以修改系统环境变量**PATH**,
将**D:\NucleiStudio\toolchain\gcc\bin;D:\NucleiStudio\toolchain\openocd\bin;D:\NucleiStudio\toolchain\qemu\bin;**增加到**PATH**中。

或者在ENV工具命令行中运行

~~~cmd
set PATH=D:\NucleiStudio\toolchain\gcc\bin;D:\NucleiStudio\toolchain\openocd\bin;D:\NucleiStudio\toolchain\qemu\bin;%PATH%
~~~

#### Linux

假设工具安装在 **~/NucleiStudio/toolchain**目录下, 通过在Linux的``.bashrc``增加如下一行代码
来添加环境变量。

~~~bash
export PATH=~/NucleiStudio/toolchain/gcc/bin:~/NucleiStudio/toolchain/openocd/bin:~/NucleiStudio/toolchain/qemu/bin:$PATH
~~~

或者在ENV工具命令行中运行

~~~bash
export PATH=~/NucleiStudio/toolchain/gcc/bin:~/NucleiStudio/toolchain/openocd/bin:~/NucleiStudio/toolchain/qemu/bin:$PATH
~~~

**注意**: 对应的RISC-V GCC和OPENOCD的路径请替换成自己安装的路径。

## 烧写及执行

### 驱动设置

驱动安装设置，请参考[Nuclei FPGA开发板介绍](https://nucleisys.com/developboard.php#ddr200t)

### 编译程序

> [!NOTE]
> 请注意由于v5.2.1移植是基于RT-Thread官方标准移植，因此中断嵌套是没有实现的，参见 https://github.com/RT-Thread/rt-thread/issues/10457

下载好[RT-Thread](https://github.com/riscv-mcu/rt-thread/issues/1)的代码和[ENV工具](https://www.rt-thread.org/document/site/#/development-tools/env/env)以后。

> 请注意测试 v5.2.1 版本，请使用 `nuclei/v5.2.1` 分支的代码。

~~~shell
# eg. for nuclei/v5.2.1 branch
git clone -b nuclei/v5.2.1 https://github.com/riscv-mcu/rt-thread.git
~~~

> **常见问题** 参见 https://github.com/riscv-mcu/rt-thread/issues/1

按照ENV工具的教程, 在**rt-thread\bsp\nuclei\nuclei_fpga_eval**目录打开ENV工具命令行。

**注意**: 请确保Nuclei GCC和Nuclei OpenOCD的路径设置正确无误。

> If you want to use Nuclei RISC-V Toolchain <= 2022.12, you need to change **PREFIX** to `riscv-nuclei-elf-` in `rtconfig.py`
>
> 如果你想测试 RT-Thread SMP模式，请按照下面的步骤进行修改。


1. 运行 ``pkgs --update``来下载最新的依赖的**Nuclei SDK**, 如果下载失败，可能需要先执行 `menuconfig` 命令来更新`.config`文件(注意需要保存)，因为依赖的SDK的位置可能被上游更新。
2. 根据需要测试运行的 Nuclei CPU Core(eg. n300fd, n900fd, nx900fd), 以及是否运行SMP模式来进行针对性的修改。
   - 修改 ``rtconfig.py`` 文件中的 ``NUCLEI_SDK_CORE``参数设置为对应的[Nuclei CPU Core(eg. n300fd, n900fd, nx900fd)](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#core)
   - 修改 ``rtconfig.py`` 文件中的 ``NUCLEI_SDK_SMP``参数设置为对应的CPU个数(eg. 2)，需要有SMP的需求，这个必须要配置。
   - 修改 ``rtconfig.py`` 文件中的 ``NUCLEI_SDK_DOWNLOAD``参数设置为不同的[运行模式](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#download)，例如下载到ILM/DLM上，下载到SRAM上，下载到DDR上。
     > 这个大小实际是由下载的Nuclei SDK Package里面的链接脚本决定，eg. ``packages\nuclei_sdk-latest\SoC\evalsoc\Board\nuclei_fpga_eval\Source\GCC\gcc_evalsoc_ilm.ld``
   - 修改 ``Kconfig`` 文件中的 ``config SOC_NUCLEI_EVALSOC`` 下面的配置，请根据你是32位还是64位来选择使能 ``ARCH_RISCV32`` 还是 ``ARCH_RISCV64``
   - 上述修改完毕后，**必须** 运行一下 ``scons --menuconfig`` 来修改配置，如果需要运行SMP则必须要配置下  `RT-Thread Kernel` -> `Enable SMP` + `Number of CPUs`选择正确SMP核心个数。
3. **务必** 运行 ``scons -c``清理之前的编译结果，然后再运行 ``scons``来编译代码, 如果电脑性能足够可以使用 ``scons -j8``来加速编译。

### 下载程序

在保证程序能够正常编译后, 在相同ENV终端执行``scons --run upload``进行代码的下载。

如果硬件在远端，则可以设置 `GDBREMOTE` 环境变量，设置远端的gdb服务器和端口，例如

~~~
# 假设gdb服务器是 192.168.31.111 端口是22800
# windows
set GDBREMOTE="192.168.31.111:22800"
# linux
export GDBREMOTE="192.168.31.111:22800"
# 然后再运行 scons --run upload 来下载代码
~~~

**注意SMP的情况下，需要确认启动openocd的时候，也需要采用多核的配置文件**。

正常下载的输出如下:

~~~
$ scons --run upload
scons: Reading SConscript files ...
Warning: Toolchain path D:/NucleiStudio/toolchain/gcc/bin doesn't exist, assume it is already in PATH
Guessed EXEC_PATH of riscv64-unknown-elf-gcc is C:\Software\NucleiStudio\toolchain\gcc\bin
Supported downloaded modes for board nuclei_fpga_eval are ('ilm', 'flash', 'flashxip', 'ddr', 'sram'), chosen downloaded mode is ilm
Newlib version:4.4.0
Nuclei SDK version: 0.8.0
Using customized GDBREMOTE "whss7:21600"
Upload application rtthread.elf using openocd and gdb
riscv64-unknown-elf-gdb rtthread.elf -ex "set remotetimeout 240"                     -ex "target remote | openocd --pipe -f C:/Work/Code/rt-thread/bsp/nuclei/nuclei_fpga_eval/packages/nuclei_sdk-latest/SoC/evalsoc/Board/nuclei_fpga_eval/openocd_evalsoc.cfg"                     --batch -ex "monitor reset halt" -ex "thread apply all info reg pc" -ex "thread 1"                     -ex "load rtthread.elf" -ex "file rtthread.elf" -ex "thread apply all set $pc=_start"                     -ex "monitor resume" -ex "quit"
0x800026c4 in rt_sem_take (sem=0x900036b8 <_timer_thread_stack+260>, timeout=-1879030596) at C:\Work\Code\rt-thread\src\ipc.c:490
490         RT_ASSERT(sem != RT_NULL);
JTAG tap: riscv.cpu tap/device found: 0x10900a6d (mfg: 0x536 (Nuclei System Technology Co Ltd), part: 0x0900, ver: 0x1)

Thread 1 (Remote target):
pc             0x800026c4       0x800026c4 <rt_sem_take+518>
[Switching to thread 1 (Remote target)]
#0  0x800026c4 in rt_sem_take (sem=0x900036b8 <_timer_thread_stack+260>, timeout=-1879030596) at C:\Work\Code\rt-thread\src\ipc.c:490
490         RT_ASSERT(sem != RT_NULL);
Loading section .init, size 0x270 lma 0x80000000
Loading section .text, size 0xb550 lma 0x80000280
Loading section .data, size 0x34a0 lma 0x90000000
Start address 0x80000140, load size 60512
Transfer rate: 89 KB/sec, 12102 bytes/write.

Thread 1 (Remote target):
A debugging session is active.

        Inferior 1 [Remote target] will be detached.

Quit anyway? (y or n) [answered Y; input not from terminal]
[Inferior 1 (Remote target) detached]
~~~

下载程序之后, 连接**串口(115200-N-8-1)**, 可以看到 RT-Thread 的输出信息:

> 最新发布出去的评估Bit一般都是16MHz或者50MHz, 串口工作在115200bps下串口读取可以正常工作.

```
 \ | /
- RT -     Thread Operating System
 / | \     5.2.1 build Jul 14 2025 15:25:28
 2006 - 2024 Copyright by RT-Thread team
```

在串口终端(我这里使用的是TeraTerm)输入``ps``即可查看当前线程工作情况:

~~~
msh />ps
thread   pri  status      sp     stack size max used left tick   error  tcb addr
-------- ---  ------- ---------- ----------  ------  ---------- ------- ------------------
tshell    20  running 0x000003c0 0x00001000    37%   0x00000008 OK      0x000000009000a7c0
sys wor   23  suspend 0x00000310 0x00000800    38%   0x0000000a OK      0x0000000090009770
tidle0   255  ready   0x00000260 0x00000400    60%   0x0000000f OK      0x0000000090005ea0
timer      4  suspend 0x000002f0 0x00001000    18%   0x00000009 EINTRPT 0x00000000900084b8
~~~

下面是 SMP 2核(nx900fd)的运行情况如下

~~~
 \ | /
- RT -     Thread Operating System
 / | \     5.2.1 build Jul 14 2025 15:34:30
 2006 - 2024 Copyright by RT-Thread team
msh />ps
thread   cpu bind pri  status      sp     stack size max used left tick   error  tcb addr
-------- --- ---- ---  ------- ---------- ----------  ------  ---------- ------- ------------------
tshell     1   2   20  running 0x000003a0 0x00001000    40%   0x00000008 OK      0x00000000a800db38
sys wor  N/A   2   23  suspend 0x00000310 0x00000800    38%   0x0000000a OK      0x00000000a800cae0
tsystem  N/A   2  254  suspend 0x00000280 0x00000400    62%   0x00000020 EINTRPT 0x00000000a8008738
tidle1   N/A   1  255  ready   0x00000220 0x00000400    69%   0x00000011 OK      0x00000000a80091e8
tidle0     0   0  255  running 0x00000200 0x00000400    61%   0x0000000b OK      0x00000000a80090b0
timer    N/A   2    4  suspend 0x000002d0 0x00001000    17%   0x00000009 EINTRPT 0x00000000a800b818
~~~

### 调试程序

在保证程序编译成功后, 在相同ENV终端执行``scons --run debug``进行代码在命令行下进行GDB调试。

正常的调试输出如下:

~~~
$ scons --run debug
scons: Reading SConscript files ...
Warning: Toolchain path D:/NucleiStudio/toolchain/gcc/bin doesn't exist, assume it is already in PATH
Guessed EXEC_PATH of riscv64-unknown-elf-gcc is C:\Software\NucleiStudio\toolchain\gcc\bin
Supported downloaded modes for board nuclei_fpga_eval are ('ilm', 'flash', 'flashxip', 'ddr', 'sram'), chosen downloaded mode is ilm
Newlib version:4.4.0
Nuclei SDK version: 0.8.0
Using customized GDBREMOTE "whss7:21600"
Debug application rtthread.elf using openocd and gdb
riscv64-unknown-elf-gdb rtthread.elf -ex "set remotetimeout 240"                     -ex "target remote | openocd --pipe -f C:/Work/Code/rt-thread/bsp/nuclei/nuclei_fpga_eval/packages/nuclei_sdk-latest/SoC/evalsoc/Board/nuclei_fpga_eval/openocd_evalsoc.cfg"
GNU gdb (GDB) 16.2.90.20250210-git
Copyright (C) 2024 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=i686-w64-mingw32 --target=riscv64-unknown-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from rtthread.elf...
Remote debugging using whss7:21600
rt_thread_idle_entry (parameter=<optimized out>) at C:\Work\Code\rt-thread\src\idle.c:269
269             for (i = 0; i < RT_IDLE_HOOK_LIST_SIZE; i++)
(gdb) info reg mhartid
mhartid        0x0      0
(gdb) bt
#0  rt_thread_idle_entry (parameter=<optimized out>) at C:\Work\Code\rt-thread\src\idle.c:269
#1  0x80002cb4 in _thread_timeout (parameter=0xdeadbeef) at C:\Work\Code\rt-thread\src\thread.c:134
#2  0x900039c0 in _timer_thread ()
Backtrace stopped: frame did not save the PC
(gdb) monitor reset halt
JTAG tap: riscv.cpu tap/device found: 0x10900a6d (mfg: 0x536 (Nuclei System Technology Co Ltd), part: 0x0900, ver: 0x1)
(gdb) load
Loading section .init, size 0x270 lma 0x80000000
Loading section .text, size 0xb550 lma 0x80000280
Loading section .data, size 0x34a0 lma 0x90000000
Start address 0x80000140, load size 60512
Transfer rate: 89 KB/sec, 12102 bytes/write.
(gdb) c
Continuing.

Program received signal SIGINT, Interrupt.
rt_thread_idle_entry (parameter=<optimized out>) at C:\Work\Code\rt-thread\src\idle.c:274
274                     idle_hook();
(gdb) info threads
  Id   Target Id         Frame
* 1    Remote target     rt_thread_idle_entry (parameter=<optimized out>) at C:\Work\Code\rt-thread\src\idle.c:274
(gdb) info reg pc
pc             0x8000134a       0x8000134a <rt_thread_idle_entry+40>
~~~

调试例子参见如下文档:

* https://doc.nucleisys.com/nuclei_sdk/quickstart.html#debug-application

为了更方便的进行调试, 也可以下载**Nuclei Studio**集成开发环境, 创建一个Debug Configuration, 选择编译好的
ELF文件, 然后配置OPENOCD和GDB即可, OPENOCD配置文件路径为

- For Nuclei SDK >= 0.8.1: **bsp\nuclei\nuclei_fpga_eval\packages\nuclei_sdk-latest\SoC\evalsoc\Board\nuclei_fpga_eval\openocd_evalsoc.cfg**


## 驱动支持情况

| 驱动 | 支持情况  |  备注  |
| ------ | ----  | :------:  |
| UART | 支持 | 蜂鸟开发板载串口是UART0 |

**注:**

- 适配RT-Thread的驱动框架的代码在 [../libraries/evalsoc/HAL_Drivers](../libraries/evalsoc/HAL_Drivers)目录下。
- 如果有开发者想适配更多的驱动, 请在对应目录下增加驱动适配支持。
- 目前串口读取统一采用中断模式进行读取，去掉了采用任务方式的查询的读取方式。

## 联系人信息

维护人:

- [fanghuaqi](https://github.com/fanghuaqi)

