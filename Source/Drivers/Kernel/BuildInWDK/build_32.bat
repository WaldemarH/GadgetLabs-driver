copy sources_32 sources /Y

build -cez

copy ".\objfre_win7_x86\i386\gadgetlabs_driver_32.pdb" "..\..\..\Build\x32\Debug\gadgetlabs_driver_32.pdb" /Y
copy ".\objfre_win7_x86\i386\gadgetlabs_driver_32.sys" "..\..\..\Build\x32\Debug\gadgetlabs_driver_32.sys" /Y
