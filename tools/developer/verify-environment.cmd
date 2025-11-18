@echo off
REM Wrapper script to run verify-environment.ps1 in CMD environment
powershell -ExecutionPolicy Bypass -File "%~dp0verify-environment.ps1"
