setlocal

call \android-ndk-r9c\ndk-build NDK_DEBUG=1

IF ERRORLEVEL 1 (
	pause
	exit /b
)

xcopy /s /y ..\install assets
del "assets\*.dll" 
call \adt-bundle-windows-x86_64-20131030\apache-ant-1.9.3\bin\ant debug install
pause
