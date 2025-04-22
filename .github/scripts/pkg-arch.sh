#!/usr/bin/env bash

if [ -z "$1" ]; then
	$0 install
	$0 build
	$0 sign
	$0 release
	exit
fi

case "$1" in
install)
	# Setup build environment
	pacman -Syu --noconfirm
	pacman -S --noconfirm sudo binutils fakeroot base-devel git meson \
		libgl libx11 libxext libxcursor libxi libxfixes libxrandr libcairomm-1.0.so \
	;;
build)
	# Fix permissions (can't makepkg as
	echo "nobody ALL=(ALL) NOPASSWD: /usr/bin/pacman" >> /etc/sudoers
	chown -R nobody .

	# Package compression settings (Matches latest Arch)
	export PKGEXT='.pkg.tar.zst'
	export COMPRESSZST=(zstd -c -T0 --ultra -20 -)

	# Build
	runuser -u nobody -- makepkg -sf --noconfirm
	;;
sign)
	if [ -z "$GPG_KEY" ] || [ -z "$GPG_KEY_ID" ]; then
		echo "WARNING: No GPG key configured, skipping signing."
		exit
	fi

	# import GPG key
	echo "$GPG_KEY" | base64 -d | gpg --import --no-tty --batch --yes
	export GPG_TTY=$(tty)

	# sign package
	ls *.pkg.tar.zst | xargs -L1 gpg --detach-sign --batch \
		--no-tty -u $GPG_KEY_ID
	;;
release)
	mkdir -p release
	mv *.pkg.tar.zst{,.sig} release
	;;
esac
