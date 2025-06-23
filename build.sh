#!/bin/bash
# See: https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
# Note: you can't refactor this out: its at the top of every script so the scripts can find their includes.
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

export METALL_CFLAGS="-I $SCRIPT_DIR/../../LLNL/metall/include"
export METALL_LIBS=" "

CFLAGS="-DBOOST_USE_SEGMENTED_STACKS -DDEBUG -ggdb -O0" CXXFLAGS="-DBOOST_USE_SEGMENTED_STACKS -DDEBUG -ggdb -O0" \
  ./configure $configureFlags --prefix=$(pwd) --disable-gc --enable-metall --disable-shared --disable-doc-gen "$@"
