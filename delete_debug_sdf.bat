@set path=c:\workspace\strat - copy

del *.sdf

cd "%path%\x64\Debug"
del . /F /Q

cd "%path%\x64\Release"
del . /F /Q

cd "%path%\Debug"
del . /F /Q

cd "%path%\Strat\x64\Debug"
del . /F /Q

cd "%path%\Strat\x64\Release"
del . /F /Q

cd "%path%\Strat\Debug"
del . /F /Q

cd "%path%\TestResults"
del . /F /Q

cd "%path%\BackTester\bin\Debug"
del . /F /Q

cd "%path%\BackTester\bin\Release"
del . /F /Q

cd "%path%\test_files"
del . /F /Q

cd "%path%\back_test_files"
del . /F /Q

cd "%path%\ipch\strat-8673b740
del . /F /Q