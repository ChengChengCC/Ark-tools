#Ark工具  ———— HeavenShadow

##关于
Windows下类似于PCHunter的ark工具

##文件结构
* bin - 可执行文件
* HeavenShadow - 应用层文件
* HeavenShadowDrv - 驱动文件

##实现功能
* 内核级进程，线程，内存，模块，窗口管理与注入功能
* 内核模块的管理
* SSDT与ShadowSSDT的hook与inline hook检测
* 关键内核模块的IAT,EAT的hook检测
* 系统IoTimer,DpcTimer和内核劳务线程的管理
* 系统内核回调的管理，例如CreateProcess、LoadImage、CreateThread等等

##开发环境
* Visual Studio 2010 + WDK 7600

##测试环境
* 本机：Windows7 x64 Ultimate SP1
* 虚拟机：VMWare + Windows7 x64 Ultimate SP1 / Windows XP SP3

##参考资料
* 《Windows内核情景分析》
* 《WindowsPE权威指南》
* 《加密与解密》
* 《Windows内核原理及实现》
* 《深入解析Windows操作系统》
*  [Windows x64驱动资料](http://bbs.pediy.com/showthread.php?t=187348)
*  [BlackBone](https://github.com/DarthTon/Blackbone)
*  wrk-v1.2源码