copy sources_64 sources /Y

build -cez

copy ".\objfre_win7_amd64\amd64\gadgetlabs_driver_32.pdb" "..\..\..\Build\x64\Debug\gadgetlabs_driver_64.pdb" /Y
copy ".\objfre_win7_amd64\amd64\gadgetlabs_driver_32.sys" "..\..\..\Build\x64\Debug\gadgetlabs_driver_64.sys" /Y
