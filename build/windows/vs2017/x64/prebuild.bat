set os_version=windows\vs2017\x64\
set dst_base=..\..\..\..\deps\
set out_base=..\..\..\..\bin\

rmdir /Q /S %dst_base%include
rmdir /Q /S %dst_base%lib

mkdir %out_base%
mkdir %dst_base%include
mkdir %dst_base%lib

::Baselib
set src_base=..\..\..\..\..\..\..\..\Versions\Baselib\

::npcap
mkdir %dst_base%include\npcap
xcopy %src_base%npcap\v1.06\%os_version%include %dst_base%include\npcap /S /Y /C
xcopy %src_base%npcap\v1.06\%os_version%lib %dst_base%lib /S /Y /C