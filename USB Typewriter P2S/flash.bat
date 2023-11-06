:start
atprogram -i isp -d atmega32u4 erase -ap || goto :fail
atprogram -i isp -d atmega32u4 program -f "Release/USB Typewriter P2S.elf" || goto :fail
atprogram -i isp -d atmega32u4 verify -f "Release/USB Typewriter P2S.elf" || goto :fail

PAUSE
goto :start

:fail
echo FAILED!
exit /b %errorlevel%

