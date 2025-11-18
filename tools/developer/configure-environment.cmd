@echo off
REM Wrapper script to run configure-environment.ps1 in CMD environment
powershell -ExecutionPolicy Bypass -File "%~dp0configure-environment.ps1"
