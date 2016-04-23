## What is that?
It's a small Windows service, made in desperate attempt to solve Intel 530 (and probably other SandForce SF-2281 based) SSDs' bug.
[See here](https://communities.intel.com/thread/46941) for details.

## What it does?
It reads random sector from specified drive (C: by default, might be changed) every specified time interval (500ms by default, might be changed),
thus preventing drive from constant cache flushing while entering power saving mode.

## How to setup?
Get x32 version [from here](https://github.com/deadbabe/diskpoll/raw/master/bin/diskpoll_x32.zip)
or x64 version [from here](https://github.com/deadbabe/diskpoll/raw/master/bin/diskpoll_x64.zip),
unzip somewhere and run **install.bat** as Administrator to install and **uninstall.bat** to uninstall.
