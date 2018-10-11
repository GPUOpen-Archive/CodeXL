@echo off

set CONFIG_NAME=Release
set D_SUFFIX=
set HYPHEN_D_SUFFIX=
set QT_DIR=%1
set QT_BIN_DIR=%2

if not EXIST %QT_DIR% set QT_DIR=..\..\Common\Lib\Ext\Qt\5.9\
if not EXIST %QT_BIN_DIR% set QT_BIN_DIR=..\..\Common\Lib\Ext\Qt\5.9\bin\win32
set QT_SRC_DIR=..\..\Common\Src\Qt\win32

call CopyFiles_AnyConfiguration.bat %CONFIG_NAME% %D_SUFFIX% %HYPHEN_D_SUFFIX%

rem Copy Qt & QScintilla Files:
XCopy /r /d /y "..\..\Common\Lib\Ext\QScintilla\2.8-GPL\lib\win32\%CONFIG_NAME%\qscintilla2.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Core.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Gui.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5OpenGL.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Network.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Xml.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Widgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebChannel.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngine.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineCore.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineWidgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5MultimediaWidgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Positioning.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5PrintSupport.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Multimedia.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sensors.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sql.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Quick.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5QuickWidgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Qml.dll" "..\Output\%CONFIG_NAME%\bin\"

rem copy other files Qt need
XCopy /r /d /y "%QT_BIN_DIR%\libEGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\libGLESv2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_DIR%\plugins\platforms\qwindows%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\platforms\"
XCopy /r /d /y "%QT_BIN_DIR%\QtWebEngineProcess.exe" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\qwebengine_convert_dict.exe" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_SRC_DIR%\qt.conf" "..\Output\%CONFIG_NAME%\bin\"
XCopy /e /i /y /s "%QT_DIR%\resources" "..\Output\%CONFIG_NAME%\bin\resources"
XCopy /e /i /y /s "%QT_DIR%\translations" "..\Output\%CONFIG_NAME%\bin\translations"

rem copy other files qcustomplot
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
