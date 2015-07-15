@cd ..

@cd .\Boot
@echo Compilando y copiando Boot
@nasm Boot.asm -t
@copy Boot ..\..\..\WORK\BOOT >> ..\..\..\noout
@del Boot

@echo Compilando y copiando Loader
@nasm Loader.asm -t
@copy Loader ..\..\..\WORK\BOOT >> ..\..\..\noout
@del Loader
@cd ..

@echo Copiando Kernel
@copy .\Kernel\Bin\%1\Kernel.x ..\..\WORK\KRNL >> ..\..\noout

@echo Copiando API
@copy .\API\Bin\%1\API.x ..\..\WORK\%2 >> ..\..\noout

@echo Copiando CLI
@copy .\CLI\Bin\%1\CLI.x ..\..\WORK\%2 >> ..\..\noout

@cd .\_all
