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
	dnf distro-sync -y
	dnf install -y rpmdevtools rpm-sign meson gcc inih-devel \
		systemd-rpm-macros rpkg
	;;
build)
	mkdir rpm
	rpkg local --outdir $PWD/rpm
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
        rpm --resign rpm/x86_64/*.rpm --define "_gpg_name $GPG_KEY_ID"
	;;
release)
	mkdir release
	mv rpm/x86_64/*.rpm release
	;;
esac
