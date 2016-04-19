@echo off
echo --- Removing .\ControlCenter\x64
rmdir ControlCenter\x64 /S /Q
echo --- Removing .\ControlCenter\GeneratedFiles 
rmdir ControlCenter\GeneratedFiles /S /Q
echo --- Removing .\ControlCenter\lastOpenNetwork.txt 
del ControlCenter\lastOpenNetwork.txt
echo --- Removing .\ControlCenter\sde-log.txt 
del ControlCenter\sde-log.txt /S /Q
echo --- Removing .\ControlCenter.sdf 
del ControlCenter.sdf /S /Q
echo --- Complete.