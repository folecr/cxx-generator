
# options

usage(){
cat << EOF
usage: $0 [options]

Test the C++ <-> JS bindings generator

OPTIONS:
-h	this help

Dependencies :
PYTHON_BIN
CLANG_ROOT
NDK_ROOT

Define this to run from a different directory
CXX_GENERATOR_ROOT

EOF
}

# exit this script if any commmand fails
set -e

# read user.cfg if it exists and is readable

_CFG_FILE=$(dirname "$0")"/user.cfg"
if [ -e "$_CFG_FILE" ]
then
    [ -r "$_CFG_FILE" ] || die "Fatal Error: $_CFG_FILE exists but is unreadable"
    . "$_CFG_FILE"
fi

# paths

if [ -z "${NDK_ROOT+aaa}" ]; then
# ... if NDK_ROOT is not set, use "$HOME/bin/android-ndk"
    NDK_ROOT="$HOME/bin/android-ndk"
fi

if [ -z "${CLANG_ROOT+aaa}" ]; then
# ... if CLANG_ROOT is not set, use "$HOME/bin/clang+llvm-3.1"
    CLANG_ROOT="$HOME/bin/clang+llvm-3.1"
fi

if [ -z "${PYTHON_BIN+aaa}" ]; then
# ... if PYTHON_BIN is not set, use "/usr/bin/python2.7"
    PYTHON_BIN="/usr/bin/python2.7"
fi

# find directory where this script lives
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# paths with defaults hardcoded to relative paths

if [ -z "${CXX_GENERATOR_ROOT+aaa}" ]; then
    CXX_GENERATOR_ROOT="$DIR/.."
fi

if [ -z "${TEST_ROOT+aaa}" ]; then
    TEST_ROOT="$CXX_GENERATOR_ROOT/test"
fi

if [ -z "${OUTPUT_ROOT+aaa}" ]; then
    OUTPUT_ROOT="$TEST_ROOT/simple_test_bindings"
fi

echo "Paths"
echo "    NDK_ROOT: $NDK_ROOT"
echo "    CLANG_ROOT: $CLANG_ROOT"
echo "    PYTHON_BIN: $PYTHON_BIN"
echo "    CXX_GENERATOR_ROOT: $CXX_GENERATOR_ROOT"
echo "    TEST_ROOT: $TEST_ROOT"

# write userconf.ini

_CONF_INI_FILE="$PWD/userconf.ini"
if [ -f "$_CONF_INI_FILE" ]
then
    rm "$_CONF_INI_FILE"
fi

_CONTENTS=""
_CONTENTS+="[DEFAULT]"'\n'
_CONTENTS+="androidndkdir=$NDK_ROOT"'\n'
_CONTENTS+="clangllvmdir=$CLANG_ROOT"'\n'
_CONTENTS+="cxxgeneratordir=$CXX_GENERATOR_ROOT"'\n'
echo 
echo "generating userconf.ini..."
echo ---
echo -e "$_CONTENTS"
echo -e "$_CONTENTS" > "$_CONF_INI_FILE"
echo ---

# Generate bindings for simpletest using Android's system headers
echo "Generating bindings for simpletest with Android headers..."
set -x
LD_LIBRARY_PATH=${CLANG_ROOT}/lib $PYTHON_BIN ${CXX_GENERATOR_ROOT}/generator.py ${TEST_ROOT}/test.ini -s testandroid -o ${OUTPUT_ROOT}
