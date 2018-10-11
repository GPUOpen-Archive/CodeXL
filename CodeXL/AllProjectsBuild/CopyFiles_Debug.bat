rem @echo off

set CONFIG_NAME=Debug
set D_SUFFIX=d
set HYPHEN_D_SUFFIX=-d

set QT_DIR=%1
set QT_BIN_DIR=%2

if not EXIST %QT_DIR% set QT_DIR=..\..\Common\Lib\Ext\Qt\5.9\
if not EXIST %QT_BIN_DIR% set QT_BIN_DIR=..\..\Common\Lib\Ext\Qt\5.9\bin\win32
set QT_SRC_DIR=..\..\Common\Src\Qt\win32

call CopyFiles_AnyConfiguration.bat %CONFIG_NAME% %D_SUFFIX% %HYPHEN_D_SUFFIX%

rem copy Qt & QScintilla files:
XCopy /r /d /y "..\..\Common\Lib\Ext\QScintilla\2.8-GPL\lib\win32\%CONFIG_NAME%\qscintilla2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Core%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Core%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Gui%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Gui%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5OpenGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5OpenGL%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Network%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Network%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Xml%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Xml%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Widgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Widgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngine%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngine%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineCore%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineCore%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebChannel%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebChannel%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineWidgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5WebEngineWidgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5MultimediaWidgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5MultimediaWidgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Positioning%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Positioning%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5PrintSupport%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5PrintSupport%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Multimedia%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Multimedia%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sensors%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sensors%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sql%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Sql%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Quick%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Quick%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5QuickWidgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5QuickWidgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Qml%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\Qt5Qml%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"

rem copy other files Qt need
XCopy /r /d /y "%QT_BIN_DIR%\libEGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\libEGL%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\libGLESv2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\libGLESv2%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_DIR%\plugins\platforms\qwindows%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\platforms\"
XCopy /r /d /y "%QT_DIR%\plugins\platforms\qwindows%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\platforms\"
XCopy /r /d /y "%QT_BIN_DIR%\QtWebEngineProcessd.exe" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_BIN_DIR%\qwebengine_convert_dict.exe" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "%QT_SRC_DIR%\qt.conf" "..\Output\%CONFIG_NAME%\bin\"
XCopy /e /i /y /s "%QT_DIR%\resources" "..\Output\%CONFIG_NAME%\bin\resources"
XCopy /e /i /y /s "%QT_DIR%\translations" "..\Output\%CONFIG_NAME%\bin\translations"


rem copy other files qcustomplot
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"