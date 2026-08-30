' Launch LibMan with Qt/MinGW runtime paths (desktop shortcut target).
Option Explicit

Dim fso, sh, root, qtDir, mingwDir, buildDir, exePath, env

Set fso = CreateObject("Scripting.FileSystemObject")
Set sh = CreateObject("WScript.Shell")

root = fso.GetParentFolderName(fso.GetParentFolderName(WScript.ScriptFullName))
qtDir = "C:\Qt\5.15.2\mingw81_64"
mingwDir = "C:\Qt\Tools\mingw810_64\bin"
buildDir = root & "\build"
exePath = buildDir & "\libman.exe"

If Not fso.FileExists(exePath) Then
    MsgBox "LibMan not found:" & vbCrLf & exePath & vbCrLf & vbCrLf & "Build the project first (Build LibMan task).", vbCritical, "LibMan"
    WScript.Quit 1
End If

Set env = sh.Environment("PROCESS")
env("PATH") = qtDir & "\bin;" & mingwDir & ";" & env("PATH")
env("QT_PLUGIN_PATH") = qtDir & "\plugins"
env("QT_QPA_PLATFORM_PLUGIN_PATH") = qtDir & "\plugins\platforms"

sh.CurrentDirectory = buildDir
sh.Run Chr(34) & exePath & Chr(34), 1, False
