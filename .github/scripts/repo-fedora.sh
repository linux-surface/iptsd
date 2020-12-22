#!/usr/bin/env bash

NAME="$1"
FEDORA="$2"

# Install dependencies
dnf install -y git findutils

repo="https://surfacebot:${SURFACEBOT_TOKEN}@github.com/linux-surface/repo.git"

# clone package repository
git clone -b "${BRANCH_STAGING}" "${repo}" repo

# copy packages
cp fedora-$FEDORA-latest/* repo/fedora/f$FEDORA
cd repo/fedora/f$FEDORA

# parse git tag from ref
GIT_TAG=$(echo $GIT_REF | sed 's|^refs/tags/||g')

# convert packages into references
for pkg in $(find . -name '*.rpm'); do
	echo "$NAME:$GIT_TAG/$(basename $pkg)" > $pkg.blob
	rm $pkg
done

# set git identity
git config --global user.email "surfacebot@users.noreply.github.com"
git config --global user.name "surfacebot"

# commit and push
rnd="$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)"
update_branch="${BRANCH_STAGING}-${rnd}"
git checkout -b "${update_branch}"
git add .
git commit -m "Update Fedora $FEDORA $NAME package"
git push --set-upstream origin "${update_branch}"
