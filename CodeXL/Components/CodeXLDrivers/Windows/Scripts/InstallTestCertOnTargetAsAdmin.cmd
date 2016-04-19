IF NOT EXIST UnitTesting.cer makecert -$ individual -r -pe -ss "CpuProf Test Store" -n CN="CpuProf Unit Test" UnitTesting.cer
certmgr.exe -add UnitTesting.cer -s -r localMachine root
certmgr.exe -add UnitTesting.cer -s -r localMachine trustedpublisher