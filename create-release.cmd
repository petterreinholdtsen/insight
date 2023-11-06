
@if "%1" EQU "" (
    @echo Usage: %0 ^<version^> 
    @goto :eof
)

@if "%QTDIR%" EQU "" (
    @echo Missing QTDIR environment variable 
    @goto :eof
) 

:: Translate
lupdate insight.pro
set /p CHANGE=If lupdate reports any changes, abort release and run linguist tool.

:: Build
nmake release

pushd src\thirdparty\create_xml
qmake
nmake
popd

set appname=insight
set app=%appname%.exe
set targetname=insight
set targetapp=%targetname%.exe
set target=innsyn-%1

rd /q/s %target%
mkdir %target%

:: APP
copy release\%app% %target%\%targetapp% 
copy release\%appname%.pdb %target%\%targetname%.pdb 
copy readme.txt %target%\.
copy lesmeg.txt %target%\.
copy %appname%.conf %target%\%targetname%.conf
copy sphinx\* %target%\.
copy src\thirdparty\pdf2text\release\pdf2text.exe %target%\.
copy c:\windows\system32\MSVCP140.DLL %target%\.
copy c:\windows\system32\MSVCR140.DLL %target%\.
copy release\*qm %target%\.
copy %QTDIR%\bin\qt6printsupport.dll %target%\.
copy %QTDIR%\bin\qt6widgets.dll %target%\.
copy %QTDIR%\bin\qt6gui.dll %target%\.
copy %QTDIR%\bin\qt6sql.dll %target%\.
copy %QTDIR%\bin\qt6core.dll %target%\.
copy %QTDIR%\bin\qt6xml.dll %target%\.
copy "c:\Program Files\MySQL\MySQL Server 5.6\lib\libmysql.dll" %target%\.

xcopy /s /i %QTDIR%\plugins\sqldrivers\*dll %target%\sqldrivers
:: Include custom built mysql driver
xcopy mysql\plugins\sqldrivers\*dll %target%\sqldrivers
xcopy /s /i %QTDIR%\plugins\platforms\*dll %target%\platforms
xcopy /s /i %QTDIR%\plugins\imageformats\*dll %target%\imageformats
xcopy /s /i %QTDIR%\plugins\styles\*dll %target%\styles
del %target%\imageformats\*d.dll
del %target%\sqldrivers\*d.dll

:: zlib
copy %ZLIBDIR%\Release\zlib.dll %target%\.
copy src\thirdparty\quazip-1.4\out\quazip\Release\*dll %target%\.

:: 7zip
copy lib\7zip\* %target%\.

:: SPHINX
set sphinxver="3.5.1"
copy lib\sphinx-%sphinxver%\bin\* %target%\.
copy src\thirdparty\create_xml\release\create_xml.exe %target%\.

:: FORMATS
xcopy /s/i formats %target%\formats

:: TESTDATA
xcopy /s/i testdata %target%\testdata

:: LOGO
copy src\gui\resources\banner_* %target%\.

set tgz=%target%.tgz
tar czf %tgz% %target%/*
