@cd..

@md ..\..\..\WORK\%2\TESTS

@echo Copiando Aplicacion de prueba
@copy .\Test\Bin\%1\Test.x ..\..\..\WORK\%2\TESTS >> ..\..\..\noout

@echo Copiando Aplicacion de prueba Multihilo
@copy .\MtTest\Bin\%1\MtTest.x ..\..\..\WORK\%2\TESTS >> ..\..\..\noout

@cd .\_all
