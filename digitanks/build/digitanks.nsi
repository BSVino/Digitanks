; The name of the installer
Name "Digitanks"

; The file to write
OutFile "digitanks.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Digitanks

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Digitanks" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

Icon "digitanks.ico"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Digitanks (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r /x .dropbox /x digitanks.pdb "\my dropbox\Digitanks\install\*.*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Digitanks "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Digitanks" "DisplayName" "Digitanks"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Digitanks" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Digitanks" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Digitanks" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Digitanks"
  CreateShortCut "$SMPROGRAMS\Digitanks\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Digitanks\Digitanks.lnk" "$INSTDIR\digitanks.exe" "" "$INSTDIR\digitanks.exe" 0
  CreateShortCut "$SMPROGRAMS\Digitanks\Digitanks (Fullscreen).lnk" "$INSTDIR\digitanks.exe" "--fullscreen" "$INSTDIR\digitanks.exe" 0

SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut"

  CreateShortCut "$DESKTOP\Digitanks.lnk" "$INSTDIR\digitanks.exe" "" "$INSTDIR\digitanks.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Digitanks"
  DeleteRegKey HKLM SOFTWARE\Digitanks

  ; Remove files and uninstaller
  RMDir /r "$INSTDIR\models"
  RMDir /r "$INSTDIR\sound"
  RMDir /r "$INSTDIR\textures"
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Digitanks\*.*"
  Delete "$DESKTOP\Digitanks.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Digitanks"
  RMDir "$INSTDIR"

SectionEnd
