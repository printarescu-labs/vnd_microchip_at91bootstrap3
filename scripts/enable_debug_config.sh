#!/bin/sh
# Turn on verbose debug output on top of an already-generated .config, for the
# "dev" CI build profile. Must run after "make <target>_defconfig" and before
# the build itself. Goes through the project's own oldconfig target so the
# derived auto.conf/autoconf.h stay consistent, rather than hand-editing
# .config and hoping the build picks it up.
set -eu

if [ ! -f .config ]; then
	echo "Error: .config not found -- run '<target>_uboot_defconfig' first" >&2
	exit 1
fi

for sym in CONFIG_DEBUG CONFIG_DEBUG_INFO; do
	if grep -q "^$sym=" .config; then
		sed -i "s/^$sym=.*/$sym=y/" .config
	elif grep -q "^# $sym is not set" .config; then
		sed -i "s/^# $sym is not set/$sym=y/" .config
	else
		echo "$sym=y" >> .config
	fi
done

make oldconfig < /dev/null
