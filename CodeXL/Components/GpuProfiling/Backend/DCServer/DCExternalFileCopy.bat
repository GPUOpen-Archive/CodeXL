: %1 = path to the GPUPerfAPI directory ($(CommonDir)\GPUPerfAPI\2_3)
: %2 = GDTBuildSuffix
: %3 = GDTPlatform
: %4 = GDTPlatformSuffix
: %5 = Destination directory for build
: %6 = Optional Destination directory for build
: %7 = Optional Destination directory for build

IF EXIST %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll ECHO Copying "GPUPerfAPIDX11%4%2.dll" into build directory
IF EXIST %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll %5
IF [%6] == [] GOTO COUNTERS
IF EXIST %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll %6
IF [%7] == [] GOTO COUNTERS
IF EXIST %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDX11%4%2.dll %7

:COUNTERS

IF EXIST %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll ECHO Copying "GPUPerfAPICounters%4%2.dll" into build directory
IF EXIST %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll %5
IF [%6] == [] GOTO DXAMDDEVICEINFO
IF EXIST %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll %6
IF [%7] == [] GOTO DXAMDDEVICEINFO
IF EXIST %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPICounters%4%2.dll %7

:DXAMDDEVICEINFO

IF EXIST %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll ECHO Copying "GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll" into build directory
IF EXIST %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll %5
IF [%6] == [] GOTO END
IF EXIST %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll %6
IF [%7] == [] GOTO END
IF EXIST %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll COPY %1\Bin%2\%3\GPUPerfAPIDXGetAMDDeviceInfo%4%2.dll %7

:END

exit 0