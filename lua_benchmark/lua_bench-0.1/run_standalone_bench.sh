#!/bin/sh
#
RUNNER=/usr/bin/lua
#RUNNER=/usr/bin/luajit
EXT=""
# use taskset to lock process to one cpu core (1 = second cpu/core)
TIME="taskset -c 1 time -f "

if [ "$1" == "native" ] ; then
	RUNNER=""
elif [ -n "$1" ] ; then
	VM=`which $1`
	if [ -x $VM ] ; then
		RUNNER=$VM
	fi
fi

if [ -n "$RUNNER" ] ; then
	EXT=".lua"
fi

# load script lists.
. ./script_lists.sh

#SCRIPT_LIST="$SCRIPT_FULL_LIST"
#SCRIPT_LIST="$SCRIPT_MEDIUM_LIST"
SCRIPT_LIST="$SCRIPT_SHORT_LIST"

script_params() {
	params=$({ cat <<EOF
$SCRIPT_LIST
EOF
} | egrep "^$1:" | sed -e 's/[^:]*:[^:]*://')
	echo ${params}
}

script_input() {
	input=$({ cat <<EOF
$SCRIPT_LIST
EOF
} | egrep "^$1:" | sed -e 's/[^:]*:\([^:]*\):.*/\1/')
	[[ -z "${input}" ]] && input="/dev/null"
	echo ${input}
}

run_test_bench() {
	# testing.
  for src in `cd bench ; ls *.lua | sed -e 's/\.lua$//'`; do
    input=`script_input $src`
    params=`script_params $src`
  	for param in ${params//,/ }; do
      echo "run script: 'bench/${src}$EXT', with arg='${param}'"
      eval "${TIME} 'time = %e'" ${RUNNER} ./bench/${src}$EXT ${param} <${input}
  	done
  done
}

run_benchmark() {
	# benchmarking.
  for src in `cd bench ; ls *.lua | sed -e 's/\.lua$//'`; do
    input=`script_input $src`
    params=`script_params $src`
  	for param in ${params//,/ }; do
      eval ${TIME} "${src}:%e" ${RUNNER} ./bench/${src}$EXT ${param} <${input} >/dev/null
  	done
  done
}

if [ -n "$RUNNER" ]; then
	echo "running benchmarks with VM: $RUNNER"
fi

# run type
if [ "$2" == "test" ] ; then
	run_test_bench
else
	run_benchmark
fi

