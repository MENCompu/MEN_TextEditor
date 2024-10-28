@echo off

set Warnings=-W4 -WX -wd4201 -wd4100 -wd4189 -wd4456 -wd4505
set PreprocessorFlags=
set CompilerOptions=-MDd -FC -Z7 -Oi -Od -GR- -EHa- -nologo
set LinkerOptions=-opt:ref -subsystem:windows,5.02
set Libraries=User32.lib Gdi32.lib Winmm.lib Shell32.lib

IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin

cl %Warnings% %PreprocessorFlags% %CompilerOptions% -LD -FmTextEditor.map ..\src\TextEditor.cpp -link    ^
   -OUT:TEPlatformIndep.dll -PDB:TextEditor_%FileNameTime%.pdb -EXPORT:KeyProcessing -EXPORT:Render             ^
   -EXPORT:UpdateSubWindowsSize -EXPORT:InitializeMemory -EXPORT:SetSubWindowsForResizing  %LinkerOptions%      ^
   %Libraries%

cl %Warnings% %PreprocessorFlags% %CompilerOptions% -FmWIN32_TextEditor.map ..\src\WIN32_TextEditor.cpp  ^
   -link -OUT:TextEditor.exe %LinkerOptions% %Libraries%
popd
