@echo off
if "%1" == "" %0 *
cd \uupc\mail
for %%i in (%1.spb) do echo %%i has mail waiting.
