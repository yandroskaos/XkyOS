@cd ..

@echo Compilando Core
@cd .\Core\_all
@call Core.bat %1 %2
@cd ..\..

@echo Compilando Commons
@cd .\Apps\Commons\_all
@call Commons.bat %1 %2
@cd ..\..\..

@echo Compilando Tests
@cd .\Apps\Tests\_all
@call Tests.bat %1 %2
@cd ..\..\..

@echo Compilando Windows
@cd .\Apps\Windows\_all
@call Windows.bat %1 %2
@cd ..\..\..

@echo Compilando Tetris
@cd .\Apps\Tetris\Project
@call Tetris.bat %1 %2
@cd ..\..\..

@cd .\_all