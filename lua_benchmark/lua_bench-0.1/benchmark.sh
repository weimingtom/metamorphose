#!/bin/sh
#
./generate_inputs.sh

taskset -c 1 time -p luajit run_bench_tests.lua >luajit.log 2>&1
taskset -c 1 time -p ./llvm-lua run_bench_tests.lua >llvm-lua.log 2>&1
taskset -c 1 time -p /usr/bin/lua run_bench_tests.lua >lua.log 2>&1
taskset -c 1 time -p ./run_standalone_bench.sh native >native.log 2>&1
lua compare_runs.lua lua.log llvm-lua.log native.log luajit.log >results.log
cat results.log

