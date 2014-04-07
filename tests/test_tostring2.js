// large hex values were causing crashes

0x8000000000000000.toString(2) // could segfault :)

result = (8446744073709551616).toString(16) == "7538dcfb76180000"
