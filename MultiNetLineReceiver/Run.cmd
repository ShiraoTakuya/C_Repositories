g++ -I./curl/include -L./curl/lib source.cpp -mwindows -lwininet -lws2_32 -fpermissive --input-charset=utf-8 -o source.exe -lcurl
PAUSE