@echo off
chcp 65001 >nul
cd /d C:\working\01_Project\FW\TimeTree

:: 1. 임시 자동 입력 키보드 스크립트(VBS) 생성
(
echo set shell = CreateObject^("WScript.Shell"^)
echo WScript.Sleep 1000
echo shell.SendKeys "oio9595@naver.com{ENTER}"
echo WScript.Sleep 500
echo shell.SendKeys "Jyds1702@mc861t{ENTER}"
echo WScript.Sleep 500
echo shell.SendKeys "1{ENTER}"
) > auto_input.vbs

:: 2. 키보드 타이핑 백그라운드 실행
start /b wscript.exe auto_input.vbs

:: 3. 원래 타임트리 프로그램 실행
timetree-exporter -o ./timetree.ics

:: 4. 끝난 후 임시 파일 삭제
del auto_input.vbs

echo.
echo =======================================
echo  TimeTree 일정 추출이 완료되었습니다!
echo =======================================
pause