#!/bin/sh
#
LLVM_LUA_DIR=../llvm-lua-0.2/llvm-lua
BENCH_DIR=`pwd`/bench

usage() {
	echo "usage: $0 <path to llvm-lua>"
	exit 0
}
if [ -d "$1" ]; then
	LLVM_LUA_DIR=$1
fi
if [ ! -d "$LLVM_LUA_DIR" ]; then
	echo "not a directory: $LLVM_LUA_DIR"
	usage
fi
LLVM_LUA="$LLVM_LUA_DIR/llvm-lua"
if [ ! -x "$LLVM_LUA" ]; then
	echo "llvm-lua not found in: $LLVM_LUA_DIR"
	usage
fi
if [ -d "$LLVM_LUA" ]; then
	echo "llvm-lua in '$LLVM_LUA_DIR' shouldn't be a directory"
	usage
fi
LUA_VM_OPS="$LLVM_LUA_DIR/lua_vm_ops.bc"
if [ ! -e "$LUA_VM_OPS" ]; then
	echo "lua_vm_ops.bc not found in: $LLVM_LUA_DIR"
	usage
fi

echo "Creating symbolic links"
ln -s ${LLVM_LUA_DIR}/llvm-lua ./
ln -s ${LLVM_LUA_DIR}/lua_vm_ops.bc ./

echo "Compiling bench/* scripts to native executables"
pushd $LLVM_LUA_DIR

./compile_all.sh $BENCH_DIR/*.lua

popd

