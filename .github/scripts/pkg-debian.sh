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
	. /etc/os-release

	# Setup build environment
	sed 's/^deb /deb-src /' /etc/apt/sources.list >> /etc/apt/sources.list
	if [ ! "$VERSION_CODENAME" = "" ]; then
		echo "deb http://deb.debian.org/debian $VERSION_CODENAME-backports main" >> /etc/apt/sources.list
	fi

	apt-get -y update
	apt-get -y install build-essential fakeroot debhelper \
		dpkg-sig git devscripts

	# Install package dependencies
	mk-build-deps -ir -t "apt-get -y -t $VERSION_CODENAME-backports"
	;;
build)
	dpkg-buildpackage -b -us -uc
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
	dpkg-sig -g "--batch --no-tty" --sign builder -k $GPG_KEY_ID ../*.deb
	;;
release)
	mkdir -p release
	mv ../*.deb release
	;;
esac
