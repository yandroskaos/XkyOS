@cd ..

@md ..\..\..\WORK\%2\WINDOWS

@echo Copiando ntldr
@copy .\ntldr\Bin\%1\ntldr.x ..\..\..\WORK\%2\WINDOWS >> ..\..\..\noout

@echo Copiando ntoskrnl
@copy .\ntoskrnl\Bin\%1\ntoskrnl.x ..\..\..\WORK\%2\WINDOWS >> ..\..\..\noout

@if %2 == XFSA goto FLOPPY
@echo Copiando Desktop
@copy .\Resources\Desktop.bmp ..\..\..\WORK\%2\WINDOWS >> ..\..\..\noout

:FLOPPY
@echo Copiando MessageBox
@copy .\Binaries\MessageBox\Release\MessageBox.exe ..\..\..\WORK\%2\WINDOWS >> ..\..\..\noout

@echo Copiando ntdll
@copy .\Binaries\ntdll\Release\ntdll.dll ..\..\..\WORK\%2\WINDOWS >> ..\..\..\noout

@cd .\_all
