# Ark-tools
Windows Ark 工具的工程和一些demo

##文件结构
* DpcTimer_X64：Windows x64 下枚举DCP定时器的小demo,之前有在看雪放出；
* DrxHook：利用调试寄存器来达到hook的目的；
* HideIDTHook：hook IDT,一种是直接hook，一种是通过修改GDT表来实现；
* Inject_By_kernelAPC：内核apc注入；
* InlineHook_ShadowSSDT：inline hook Shadow SSDT；
* Register：注册表编辑器，和regedit.exe功能类似，通过驱动实现；
* Wow64Injectx64：32位进程注入64位进程；
* arkProject：ark工具的完整项目源码；
