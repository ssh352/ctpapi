set GYP_MSVS_VERSION=2015
SET GYP_GENERATORS=msvs
REM set GYP_DEFINES= component=shared_library enable_protobuf_log=1
REM set GYP_DEFINES= component=static_library enable_protobuf_log=0
set GYP_DEFINES= component=static_library target_arch=x64
REM set GYP_DEFINES= component=static_library caf_use_asio=1 caf_log_level=1 caf_no_exceptions=1 caf_enable_runtime_checks=1 caf_no_mem_management=1
REM set GYP_DEFINES= component=shared_library target_arch=x64 

REM python gyp_hello.py --depth=.. -Icommon.gypi -Dhost_arch=x64 -Dtarget_arch=x64 -Dclang=0 -Dasan=0 %~dp0..\ispread\lhsnet_demo.gyp
REM python gyp_hello.py --depth=.. -Icommon.gypi -Dclang=0 -Dasan=0 %~dp0..\ispread\lhsnet_demo.gyp
REM python %~dp0gyp_hello.py --depth=.. -I%~dp0common.gypi -D"clang=0 asan=0" %~dp0..\ispread\lhsnet_demo.gyp
REM %~dp0\tools\python276_bin\python build\gyp_win.py --depth=. --no-circular-check -Ibuild\common.gypi build\some.gyp
python build\gyp_win.py --depth=. --no-circular-check -Ibuild\common.gypi geek_quant\geek_quant.gyp
