#define AppName "LibMan"
#define AppVersion "1.0.0"
#define AppExe "libman.exe"
#define AppIco "libman.ico"

[Setup]
AppId={{B7F0E1A2-3C44-4D8F-9C2B-1A6F12345678}}
AppName={#AppName}
AppVersion={#AppVersion}
DefaultDirName={pf}\LibMan
DefaultGroupName=LibMan
OutputBaseFilename=LibMan-Setup
Compression=lzma
SolidCompression=yes
SetupIconFile={#AppIco}
UninstallDisplayIcon={app}\{#AppIco}
ChangesEnvironment=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
; Main application + Qt runtime + everything staged into build\dist
Source: "..\build\dist\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

; Application icon
Source: "{#AppIco}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
; Start Menu shortcut
Name: "{group}\LibMan"; \
    Filename: "{app}\{#AppExe}"; \
    IconFilename: "{app}\{#AppIco}"

; Desktop shortcut
Name: "{autodesktop}\LibMan"; \
    Filename: "{app}\{#AppExe}"; \
    IconFilename: "{app}\{#AppIco}"; \
    Tasks: desktopicon

; Uninstall shortcut
Name: "{group}\Uninstall LibMan"; \
    Filename: "{uninstallexe}"; \
    IconFilename: "{app}\{#AppIco}"

[Tasks]
Name: "desktopicon"; \
    Description: "Create desktop icon"; \
    GroupDescription: "Additional icons:"

Name: "addtopath"; \
    Description: "Add LibMan to PATH (recommended)"; \
    GroupDescription: "System integration:"; \
    Flags: checkedonce

[Registry]
; Add install dir to system PATH
Root: HKLM; \
Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
ValueType: expandsz; \
ValueName: "Path"; \
ValueData: "{olddata};{app}"; \
Tasks: addtopath; \
Check: NeedsAddPath

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
function NeedsAddPath(): Boolean;
var
  Path: string;
begin
  if not RegQueryStringValue(
    HKLM,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path',
    Path
  ) then
  begin
    Result := True;
    exit;
  end;

  Result := Pos(
    Lowercase(ExpandConstant('{app}')),
    Lowercase(Path)
  ) = 0;
end;