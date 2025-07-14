# taintAnalysisDemo
LLVM based pass to track taints from source to sink

Build LLVM using cmake or Visual Studio.

Run clang to generate file with .ll extension for testing pass manager

clang -O1 -S -emit-llvm test_taint.c -o test_taint.ll

Next run opt with your pass 

./build/Debug/bin/opt.exe -disable-output test_taint.ll -passes=taintanalyze

Below is sample output

Found taint source user_input in call instruction in function main

[WARNING] Tainted variable  user_input reaching sink in call instruction in function main 


