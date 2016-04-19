@echo off

set CONFIG_NAME=Debug
set D_SUFFIX=d
set HYPHEN_D_SUFFIX=-d

call CopyFiles_AnyConfiguration.bat %CONFIG_NAME% %D_SUFFIX% %HYPHEN_D_SUFFIX%

rem copy Qt & QScintilla files:
XCopy /r /d /y "..\..\Common\Lib\Ext\QScintilla\2.8-GPL\lib\win32\%CONFIG_NAME%\qscintilla2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Core%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Core%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Gui%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Gui%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5OpenGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5OpenGL%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Network%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Network%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Xml%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Xml%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Widgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Widgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKit%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKit%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebChannel%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebChannel%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKitWidgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5WebKitWidgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5MultimediaWidgets%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5MultimediaWidgets%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Positioning%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Positioning%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5PrintSupport%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5PrintSupport%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Multimedia%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Multimedia%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sensors%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sensors%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sql%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Sql%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Quick%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Quick%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Qml%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\Qt5Qml%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"

rem copy other files Qt need
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icudt54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icuin54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\icuuc54.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libEGL%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libEGL%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libGLESv2%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\bin\win32\libGLESv2%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\plugins\platforms\qwindows%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\platforms\"
XCopy /r /d /y "..\..\Common\Lib\Ext\Qt\5.5\plugins\platforms\qwindows%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\platforms\"


rem copy other files qcustomplot
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\qcustomplot\1.3.1\lib\Win32\%CONFIG_NAME%\qcustomplot%D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
