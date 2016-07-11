: %1 = path to the GPUPerfAPI directory ($(CommonDir)\GPUPerfAPI\2_3)
: %2 = GDTBuildSuffix
: %3 = GDTPlatform
: %4 = GDTPlatformSuffix
: %5 = Destination directory for build
: %6 = Optional Destination directory for build
: %7 = Optional Destination directory for build

: There is no NDA version of GPUPerfAPI, so map NDA to Public version
set BUILDSUFFIX=%2
IF [%BUILDSUFFIX%] == ["-nda"] set BUILDSUFFIX=

IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll ECHO Copying "GPUPerfAPIHSA%4%BUILDSUFFIX%.dll" into build directory
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll %5
IF [%6] == [] GOTO COUNTERS
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll %6
IF [%7] == [] GOTO COUNTERS
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPIHSA%4%BUILDSUFFIX%.dll %7

:COUNTERS

IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll ECHO Copying "GPUPerfAPICounters%4%BUILDSUFFIX%.dll" into build directory
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll %5
IF [%6] == [] GOTO END
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll %6
IF [%7] == [] GOTO END
IF EXIST %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll COPY %1\Bin%BUILDSUFFIX%\%3\GPUPerfAPICounters%4%BUILDSUFFIX%.dll %7

:END

exit 0