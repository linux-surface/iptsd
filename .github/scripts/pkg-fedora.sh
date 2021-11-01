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
	dnf distro-sync -y
	dnf install -y rpmdevtools rpm-sign rpkg python3-setuptools 'dnf-command(builddep)'

	# Install package dependencies
	dnf builddep -y *.spec
	;;
build)
	mkdir rpm

	# Make sure that we have a git repository
	if [ ! -d ".git" ]; then
		git init
		git add .
	fi

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
        rpm --resign rpm/**/*.rpm --define "_gpg_name $GPG_KEY_ID"
	;;
release)
	mkdir -p release
	mv rpm/**/*.rpm release
	;;
esac
