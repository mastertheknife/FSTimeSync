*** FS Time Sync v1.0 - 25th May 2010 ***

FSTimeSync is a free open source program designed to synchronize the time and date of Microsoft's Flight Simulator into real time. It works by synchronizing the UTC time of the simulator with the UTC time of the system.
FSTimeSync should work with the following versions with Microsoft's Flight Simulator, although not tested in all: FS98, FS2000, FS2002, FS2004 and FSX.
The program was tested extensively to be working well with FS2004, but FS X has some unresolved issues which seem to come from FSUIPC.
In the future i might try to solve those issues or bypass them, perhaps by using Microsoft's SimConnect() interface to synchronize the time, so for now know that FS X support is experimental.

System requirements: Windows 2000 or newer, FS98 and newer with FSUIPC.
FSUIPC is free and can be downloaded here: http://www.schiratti.com/dowson.html


The program is open source and licensed with the General Public License version 3 which can be found in gpl.txt
The source can be found in Github's project page below. To compile the source the files FSUIPC_User.h and FSUIPC_User.lib from the FSUIPC SDK are needed.
Inside the source there is also a project file available for the Dev-C++ IDE which can be downloaded for free. Although i'm not using Visual C++, it shouldn't be hard to get it compile this program. I suppose all needed is to create a new project, add all files and tell the linker to link with comctl32.lib and FSUIPC_User.lib and it should hopefully compile flawlessy.


The project's page on Github:
http://github.com/mastertheknife/FSTimeSync
Please feel free to send suggestions, feature requests, modifications and bug reports to me through the project's page.


I hope you will enjoy using the program as much as i enjoyed writing it.
My email: mastertheknife at gmail dot com

Kfir Itzhak
