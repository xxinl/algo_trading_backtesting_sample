del *.sdf

set folder=".\Debug"
cd %folder%
del . /F /Q

set folder="..\Strat\Debug"
cd %folder%
del . /F /Q

set folder="..\..\Demo\Debug"
cd %folder%
del . /F /Q

set folder="..\..\Release"
cd %folder%
del . /F /Q

set folder="..\Strat\Release"
cd %folder%
del . /F /Q

set folder="..\..\Demo\Release"
cd %folder%
del . /F /Q