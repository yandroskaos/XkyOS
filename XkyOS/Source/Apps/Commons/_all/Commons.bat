@cd..

@md ..\..\..\WORK\%2\COMMONS

@echo Copiando User Mode Run Time Library
@copy .\RTL\Bin\%1\RTL.x ..\..\..\WORK\%2\COMMONS >> ..\..\..\noout

@cd .\_all
