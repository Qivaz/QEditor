copy ..\..\build-QEditor-Desktop_Qt_6_5_3_MinGW_64_bit-Release\release\QEditor.exe .\

windeployqt .\QEditor.exe --dir .\QEditor_windows\

copy ..\zh_CN.qm .\QEditor_windows\
copy ..\third_party\qssh\lib\QSsh.dll .\QEditor_windows\

copy .\QEditor.exe .\QEditor_windows\
cd .\QEditor_windows\

del D3Dcompiler_47.dll
del libEGL.dll
del libGLESv2.dll
del opengl32sw.dll
:: icon del Qt5Svg.dll

rd /s/q bearer
rd /s/q iconengines
del .\imageformats\qgif.dll
del .\imageformats\qico.dll
:: icon del .\imageformats\qsvg.dll
del .\imageformats\qtiff.dll
del .\imageformats\qwebp.dll
del .\imageformats\qicns.dll
del .\imageformats\qjpeg.dll
del .\imageformats\qtga.dll
del .\imageformats\qwbmp.dll
rd /s/q styles
rd /s/q translations

cd ..

echo Deploy QEditor finished.