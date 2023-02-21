#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

## unload IPTSDaemon
sudo launchctl unload /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null
# old config folder (will be removed later)
sudo rm -rf /usr/local/ipts_config 2>/dev/null

# install new/updated IPTSDaemon and set permissions
sudo mkdir -p /usr/local/bin/
sudo cp $DIR/IPTSDaemon /usr/local/bin/
sudo chmod 755 /usr/local/bin/IPTSDaemon
sudo chown root:wheel /usr/local/bin/IPTSDaemon
sudo xattr -d com.apple.quarantine /usr/local/bin/IPTSDaemon 2>/dev/null

# install config files for devices
if [ ! -d "/usr/local/iptsd/" ]; then
    sudo mkdir /usr/local/iptsd/
    sudo cp -r $DIR/presets /usr/local/iptsd/
    sudo cp -r $DIR/iptsd.conf /usr/local/iptsd/
else
    echo "Do you need to overwrite config files? [y]/n"
    read ans
    if [ -z "$ans" ] || [ $ans == "Y" ] || [ $ans == "y" ]; then
        sudo cp -r $DIR/presets /usr/local/iptsd/
        sudo cp -r $DIR/iptsd.conf /usr/local/iptsd/
    fi
fi


# install launcher for IPTSDaemon at boot
sudo cp $DIR/com.xavier.IPTSDaemon.plist /Library/LaunchDaemons
sudo chmod 644 /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
sudo chown root:wheel /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
sudo xattr -d com.apple.quarantine /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null

sudo launchctl load /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist
