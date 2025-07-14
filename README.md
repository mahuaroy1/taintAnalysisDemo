# taintAnalysisDemo
LLVM based pass to track taints from source to sink

// Run clang to generate file with .ll extension for testing pass manager
clang -O1 -S -emit-llvm test_taint.c -o test_taint.ll
// Run opt with your pass 
./build/Debug/bin/opt.exe -disable-output test_taint.ll -passes=taintanalyze

// Sample output
Found taint source user_input in call instruction in function main
[WARNING] Tainted variable  user_input reaching sink in call instruction in function main 


