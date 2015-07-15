@cd ..
@echo Preparando XkyOS
@cd .\Source\_all
@call XkyOS.bat %1 %2
@cd ..\..

@echo Copiando contenidos del disco
@cd .\WORK
@rd .\XFS /S /Q
@md .\XFS
@xcopy .\%2\* .\XFS\ /E /Q >> ..\noout

@echo Creando disco
@if %2 == XFSC goto DISK
@if %2 == XFSA goto FLOPPY

:DISK
@del xfsc.dsk /Q
@del xfsc.vmdk /Q
@copy ..\xfsc.dsk . >> ..\noout
@XFS . xfsc.dsk
@copy xfsc.dsk "C:\Archivos de programa\Bochs-2.3" >> ..\noout
@qemu-img convert -f raw xfsc.dsk -O vmdk xfsc.vmdk
@copy xfsc.vmdk "D:\Virtual Machines\XkyOS" >> ..\noout
@goto END

:FLOPPY
@del xfsa.dsk /Q
@XFS . xfsa.dsk
@copy xfsa.dsk "D:\Virtual Machines\XkyOS" >> ..\noout
@copy xfsa.dsk "C:\Archivos de programa\Bochs-2.3" >> ..\noout

:END
@cd ..
@del noout
@pause
