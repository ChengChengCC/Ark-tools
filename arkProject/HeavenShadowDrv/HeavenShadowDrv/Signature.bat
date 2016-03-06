rem 该批处理为64位驱动签名
rem @echo off

set iPath=%1
set iDate=%date:~0,10%

echo 尝试进行驱动签名...

if exist "C:\Windows\CSignTool.exe" goto SIGN

echo 未找到签名工具，放弃签名！
goto END

:SIGN
echo.

echo 修改系统日期为 2011/11/11
date 2011/11/11

CSignTool.exe sign /r iDriverSign /f %iPath% /ac

echo 恢复系统日期到 %iDate%
date %iDate%

:END

	








