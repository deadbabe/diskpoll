@echo off
sc create DiskPoll binPath= "%~dp0diskpoll.exe C: 500" type= own start= auto obj= "NT AUTHORITY\SYSTEM"
sc start DiskPoll
