setlocal

call \android-ndk-r9c\ndk-build NDK_DEBUG=1

IF ERRORLEVEL 1 (
	pause
	exit /b
)

xcopy /s /y D:\Dropbox\Digitanks\install assets
del "assets\*.dll" 
del "assets\*.exe" 
call \adt-bundle-windows-x86_64-20131030\apache-ant-1.9.3\bin\ant debug install
pause
