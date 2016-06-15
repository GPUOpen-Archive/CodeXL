@echo off

set CONFIG_NAME=Release
set D_SUFFIX=
set HYPHEN_D_SUFFIX=

call CopyFiles_AnyConfiguration.bat %CONFIG_NAME% %D_SUFFIX% %HYPHEN_D_SUFFIX%

rem Copy Qt & QScintilla Files:
XCopy /r /d /y "..\..\Common\Lib\Ext\QScintilla\2.8-GPL\lib\win32\%CONFIG_NAME%\qscintilla2.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Core.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Gui.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5OpenGL.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Network.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Xml.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Widgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebChannel.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKit.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKitWidgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5MultimediaWidgets.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Positioning.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5PrintSupport.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Multimedia.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sensors.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sql.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Quick.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Qml.dll" "..\Output\%CONFIG_NAME%\bin\"

rem copy other files Qt need
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icudt54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icuin54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icuuc54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libEGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libGLESv2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\plugins\platforms\qwindows%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\platforms\"

rem copy other files qcustomplot
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
