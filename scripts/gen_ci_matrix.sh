#!/bin/sh
# Discover buildable (board, medium) pairs from contrib/board/vd-rd/*, cross
# them with the given build profiles, and emit a JSON array suitable for a
# GitHub Actions `strategy.matrix.include`.
#
# The (board, medium) pairs are derived directly from the *_uboot_defconfig
# files that exist on disk, not hand-maintained -- a board/medium combo with
# no defconfig can never appear in the matrix, and a new board or defconfig
# is picked up automatically.
#
# Usage: gen_ci_matrix.sh '["release","dev"]'
set -eu

VDRD_DIR="contrib/board/vd-rd"
profiles_json="${1:?usage: gen_ci_matrix.sh '[\"release\",\"dev\"]'}"

pairs='[]'
for board_dir in "$VDRD_DIR"/*/; do
	[ -d "$board_dir" ] || continue
	board=$(basename "$board_dir")
	for defcfg in "$board_dir"*_uboot_defconfig; do
		[ -f "$defcfg" ] || continue
		name=$(basename "$defcfg")
		# "glasnost_m9g10sd_uboot_defconfig" -> board "glasnost_m9g10", medium "sd"
		medium=${name#"$board"}
		medium=${medium%_uboot_defconfig}
		[ -n "$medium" ] || continue
		pairs=$(printf '%s' "$pairs" | jq -c \
			--arg board "$board" --arg medium "$medium" \
			'. + [{"board":$board,"medium":$medium}]')
	done
done

printf '%s' "$pairs" | jq -c --argjson profiles "$profiles_json" \
	'[ .[] as $p | $profiles[] as $pr | $p + {"profile":$pr} ]'
