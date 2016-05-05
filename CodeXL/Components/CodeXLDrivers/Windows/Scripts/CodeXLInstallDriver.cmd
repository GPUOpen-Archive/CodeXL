:: If CodeXL is not installed, this script can be executed to installed
:: the required driver libraries for Cpu and Power profiling.    

@echo off
setlocal enableextensions

:: to execute this script admin rights are required
goto verify_admin_privledge

:verify_admin_privledge
    echo Script needs to be executed in admnin mode, detecting adminstrative permissions...

    net session >nul 2>&1
    if %errorLevel% == 0 (
        echo Adminstrative permissions confirmed.
        goto stop_services
    ) else (
        echo Exiting, please open the command prompt with Administrative permissions and rerun this script.
        pause >nul
        goto :EOF
    )

:stop_services
	echo.
	echo.
	echo Stopping CodeXLDriversLoadService service...

	sc stop CodeXLDriversLoadService >nul 2>&1
	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService service stoppped.
	) else 	(
		echo CodeXLDriversLoadService has not been started.
	)

	echo Deleting CodeXLDriversLoadService service...
	sc delete CodeXLDriversLoadService >nul 2>&1
	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService service deleted.
	) else 	(
		echo CodeXLDriversLoadService does not exists as an installed service
	)

	echo Stopping CpuProf service...
	sc stop cpuprof >nul 2>&1
	if %errorLevel% == 0 (
		echo CpuProf service stoppped.
	) else 	(
		echo CpuProf services has not been started.
	)

	echo Stopping AmdtPwrProf service...
	sc stop amdtpwrprof >nul 2>&1
	if %errorLevel% == 0 (
		echo AmdtPwrProf service stoppped.
	) else 	(
		echo AmdtPwrProf services has not been started.
	)

	echo Stopping pcore service...
	sc stop pcore >nul 2>&1
	if %errorLevel% == 0 (
		echo pcore service stoppped.
	) else 	(
		echo pcore services has not been started.
	)

	goto delete_services
   	
:delete_services
	echo.
	echo.
	echo Deleting CpuProf service...
	sc delete cpuprof >nul 2>&1
	if %errorLevel% == 0 (
		echo CpuProf service deleted.
	) else 	(
		echo Cpuprof does not exists as an installed service.
	)

	echo Deleting AmdtPwrProf service...
	sc delete amdtpwrprof >nul 2>&1
	if %errorLevel% == 0 (
		echo AmdtPwrProf service deleted.
	) else 	(
		echo AmdtPwrProf does not exists as an installed service
	)

	echo Deleting Pcore service...
	sc delete pcore >nul 2>&1
	if %errorLevel% == 0 (
		echo Pcore service deleted.
	) else 	(
		echo Pcore does not exists as an installed service
	)

	goto delete_files

     	
: delete_files 
   	echo.
   	echo.
   	echo Deleting installed service files from C:\Windows\System32\drivers directory
	set folder=""%Systemroot%\System32\drivers"
	pushd %folder% 
	del /s CpuProf.* >nul 2>&1
	del /s AMDTPwrProf.* >nul 2>&1
	del /s pcore.* >nul 2>&1
	popd
	goto copy_libs

:copy_libs
   	echo.
   	echo.
   	echo Copying requireds system file to C:\Windows\System32\drivers
	reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86"  && set OS=32BIT || set OS=64BIT > nul 2>&1
	set path=..\lib
	set folder="%Systemroot%\System32\drivers"
	
	if %OS%==32BIT (
		set release=x86
	) 
	if %OS%==64BIT (
		set release=x64
	)
   
   	set complete_path=%path%\%release%   	
   	pushd %complete_path%
	copy CpuProf.* >nul %folder%\. 2>&1
	copy AMDTPwrProf.* %folder%\. >nul 2>&1
	copy pcore.* %folder%\. >nul 2>&1
   	popd
   	goto create_servcice 
   
:create_servcice
	set system32=""%Systemroot%\System32"
	echo Creating and starting CodeXLDriversLoadService which will creates and start pcore, amdtpwrprof and cpuprof services.
	pushd %system32%
	set script_path=%~dp0
	if %release%==x64 (
		set bin_path=%script_path%..\bin\CodeXLDriversLoadService-x64.exe 
	) else (
		set bin_path=%script_path%..\bin\CodeXLDriversLoadService.exe 
	)
	sc.exe create CodeXLDriversLoadService type= own binPath= %bin_path%   >nul 2>&1
	sc.exe start CodeXLDriversLoadService >nul 2>&1
	popd
	goto verify_service
		
:verify_service
	echo.
	echo.
	echo Verifing CodeXLDriversLoadService, pcore, amdtpwrprof and cpuprof services....
	set ret=0
	pushd %system32%
	timeout /t 5 > nul
	sc.exe query CodeXLDriversLoadService | findstr "STATE" | findstr "RUNNING"  >nul 2>&1

	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService is running.
	) else 	(
		call :service_failed CodeXLDriversLoadService
	)  

	sc.exe query pcore | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo pcore is running.
	) else 	(
		call :service_failed pcore
	)  

	sc.exe query amdtpwrprof | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo amdtpwrprof is running.
	) else 	(
		call :service_failed amdtpwrprof 
	)  

	sc.exe query cpuprof | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo cpuprof is running.
	) else 	(
		call :service_failed cpuprof
	)  
     
	goto :EOF	

:service_failed
	echo %* is not running. Please reboot your machine.
	popd
	exit /B 0
