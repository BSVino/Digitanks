@echo off
..\ext-deps\protobuf-2.5.0\bin\protoc.exe --cpp_out=. common/protobuf/math.proto
..\ext-deps\protobuf-2.5.0\bin\protoc.exe --cpp_out=. toys/toy.proto
pause
