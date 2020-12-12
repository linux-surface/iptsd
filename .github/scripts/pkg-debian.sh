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
	sed 's/^deb /deb-src /' /etc/apt/sources.list >> /etc/apt/sources.list
	apt-get -y update
	apt-get -y install build-essential fakeroot debhelper dpkg-sig \
		git meson libinih-dev pkg-config systemd udev
	;;
build)
	dpkg-buildpackage -b -us -uc
	;;
sign)
	# import GPG key
	echo "$GPG_KEY" | base64 -d | gpg --import --no-tty --batch --yes
	export GPG_TTY=$(tty)

	# sign package
	dpkg-sig -g "--batch --no-tty" --sign builder -k $GPG_KEY_ID ../*.deb
	;;
release)
	mkdir release
	mv ../*.deb release
	;;
esac
