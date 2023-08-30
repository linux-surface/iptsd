#!/usr/bin/env bash

NAME="$1"

# Install dependencies
pacman -Syu --noconfirm
pacman -S --noconfirm git

repo="https://surfacebot:${SURFACEBOT_TOKEN}@github.com/linux-surface/repo.git"

# clone package repository
git clone -b "${BRANCH_STAGING}" "${repo}" repo

# copy packages
cp arch-latest/* repo/arch/
cd repo/arch

# parse git tag from ref
GIT_TAG=$(echo $GIT_REF | sed 's|^refs/tags/||g')

# convert packages into references
for pkg in $(find . -name '*.pkg.tar.zst'); do
	echo "$NAME:$GIT_TAG/$(basename $pkg)" > $pkg.blob
	rm $pkg
done

# set git identity
git config --global user.email "surfacebot@users.noreply.github.com"
git config --global user.name "surfacebot"

# commit and push
rnd="$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)"
update_branch="${BRANCH_STAGING}-${rnd}"
git switch -c "${update_branch}"
git add .
git commit -m "Update Arch Linux $NAME package"
git push --set-upstream origin "${update_branch}"
