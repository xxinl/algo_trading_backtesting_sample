del *.sdf

set folder=".\x64\Debug"
cd %folder%
del . /F /Q

set folder="..\Release"
cd %folder%
del . /F /Q

set folder="..\..\Demo\x64\Debug"
cd %folder%
del . /F /Q

set folder="..\Release"
cd %folder%
del . /F /Q

set folder="..\..\..\Strat\x64\Debug"
cd %folder%
del . /F /Q

set folder="..\Release"
cd %folder%
del . /F /Q

set folder="..\..\..\TestResults"
cd %folder%
del . /F /Q