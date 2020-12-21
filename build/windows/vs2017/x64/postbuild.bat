set os_version=windows\vs2017\x64\
set dst_base=..\..\..\..\..\..\..\..\Versions\Baselib\PlatformBase\v1.0.0\%os_version%

md %dst_base%
rmdir /Q /S %dst_base%include
rmdir /Q /S %dst_base%lib
mkdir %dst_base%include
mkdir %dst_base%lib

xcopy ..\..\..\..\include %dst_base%include /S /Y /C
xcopy ..\..\..\..\bin %dst_base%lib /S /Y /C