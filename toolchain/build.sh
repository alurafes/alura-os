#!/usr/bin/env bash
set -euo pipefail

# this script was assisted by claude, i barely did any bash scripts

PREFIX="$HOME/opt/cross-alura"
TARGET=i686-elf
SRC="$PREFIX/src"
JOBS=$(nproc)

export PATH="$PREFIX/bin:$PATH"

BINUTILS_VER=2.47
GCC_VER=16.2.0
NEWLIB_VER=4.6.0.20260123

mkdir -p "$SRC"
cd "$SRC"

log() { echo "=== $(date '+%H:%M:%S') $* ==="; }

[ -f "binutils-$BINUTILS_VER.tar.xz" ] || curl -sSL -o "binutils-$BINUTILS_VER.tar.xz" "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz"
[ -f "gcc-$GCC_VER.tar.xz" ] || curl -sSL -o "gcc-$GCC_VER.tar.xz" "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz"
[ -f "newlib-$NEWLIB_VER.tar.gz" ] || curl -sSL -o "newlib-$NEWLIB_VER.tar.gz" "https://sourceware.org/pub/newlib/newlib-$NEWLIB_VER.tar.gz"
log "downloads done"

[ -d "binutils-$BINUTILS_VER" ] || tar xf "binutils-$BINUTILS_VER.tar.xz"
[ -d "gcc-$GCC_VER" ] || tar xf "gcc-$GCC_VER.tar.xz"
[ -d "newlib-$NEWLIB_VER" ] || tar xf "newlib-$NEWLIB_VER.tar.gz"
log "extraction done"

cd "$SRC/gcc-$GCC_VER"
./contrib/download_prerequisites
cd "$SRC"
log "gcc prerequisites fetched"

mkdir -p "$SRC/build-binutils"
cd "$SRC/build-binutils"
../"binutils-$BINUTILS_VER"/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j"$JOBS"
make install
log "binutils installed"

mkdir -p "$SRC/build-gcc"
cd "$SRC/build-gcc"
../"gcc-$GCC_VER"/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers --with-newlib --disable-shared --disable-threads --disable-libssp --disable-libgomp --disable-libmudflap
make -j"$JOBS" all-gcc
make -j"$JOBS" all-target-libgcc
make install-gcc
make install-target-libgcc
log "gcc installed"

mkdir -p "$SRC/build-newlib"
cd "$SRC/build-newlib"
../"newlib-$NEWLIB_VER"/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-newlib-supplied-syscalls --disable-libgloss
make -j"$JOBS"
make install
log "newlib installed"

log "$PREFIX/bin is ready to use"
