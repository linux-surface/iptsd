#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

## unload IPTSDaemon
sudo launchctl unload /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null
## remove the files
sudo rm /usr/local/bin/IPTSDaemon 2>/dev/null
sudo rm /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null
sudo rm -rf /usr/local/ipts_config 2>/dev/null

# install new/updated IPTSDaemon and set permissions
sudo mkdir -p /usr/local/bin/
sudo chmod -R 755 /usr/local/bin/
sudo cp $DIR/IPTSDaemon /usr/local/bin/
sudo chmod 755 /usr/local/bin/IPTSDaemon
sudo chown root:wheel /usr/local/bin/IPTSDaemon
sudo xattr -d com.apple.quarantine /usr/local/bin/IPTSDaemon 2>/dev/null

# install config files for devices
sudo cp -r $DIR/ipts_config /usr/local/

# install launcher for IPTSDaemon at boot
sudo cp $DIR/com.xavier.IPTSDaemon.plist /Library/LaunchDaemons
sudo chmod 644 /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
sudo chown root:wheel /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
sudo xattr -d com.apple.quarantine /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null

sudo launchctl load /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
