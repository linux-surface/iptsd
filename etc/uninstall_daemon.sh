#!/bin/bash

## unload IPTSDaemon
sudo launchctl unload /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null
## remove the files
sudo rm /usr/local/bin/IPTSDaemon 2>/dev/null
sudo rm /Library/LaunchDaemons/com.xavier.IPTSDaemon.plist 2>/dev/null
# old config files
sudo rm -rf /usr/local/ipts_config 2>/dev/null

echo "Do you need to also remove config files? [y]/n"
read ans
if [ -z "$ans" ] || [ $ans == "Y" ] || [ $ans == "y" ]; then
    sudo rm -rf /usr/local/iptsd 2>/dev/null
fi
