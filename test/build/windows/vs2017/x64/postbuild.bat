set os_version=windows\vs2017\x64\
set out_base=..\..\..\..\bin\

::Baselib
set src_base=..\..\..\..\..\..\..\..\..\Versions\Baselib\

::PlatformBase
xcopy %src_base%PlatformBase\v1.0.0\%os_version%lib %out_base% /S /Y /C