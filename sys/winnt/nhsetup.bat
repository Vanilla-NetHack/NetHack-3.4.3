@REM  SCCS Id: @(#)nhsetup.bat      96/02/15                
@REM  Copyright (c) NetHack PC Development Team 1993, 1996
@REM  NetHack may be freely redistributed.  See license for details. 
@REM  Win32 setup batch file, see Install.nt for details
@REM
@echo off
echo Checking to see if directories are set up properly
if not exist ..\..\include\hack.h goto err_dir
if not exist ..\..\src\hack.c goto err_dir
if not exist ..\..\dat\wizard.des goto err_dir
if not exist ..\..\util\makedefs.c goto err_dir
if not exist ..\..\sys\winnt\winnt.c goto err_dir
echo Directories look ok.

:do_rest
echo "Copying Makefile.NT to ..\..\src\Makefile"
copy makefile.NT ..\..\src\Makefile >nul
echo Makefile copied ok.

if exist ..\..\dat\data.bas goto long1ok
if exist ..\..\dat\data.base goto long1a
if exist ..\..\dat\data~1.bas goto long1b
goto err_data

:long1a
echo Changing some long-named distribution file names:
echo "Copying ..\..\dat\data.base -> ..\..\dat\data.bas"
copy ..\..\dat\data.base ..\..\dat\data.bas
if exist ..\..\dat\data.old del ..\..\dat\data.old
ren ..\..\dat\data.base data.old
goto long1ok

:long1b
echo Changing some long-named distribution file names:
echo "Copying ..\..\dat\data~1.bas -> ..\..\dat\data.bas"
copy ..\..\dat\data~1.bas ..\..\dat\data.bas
if exist ..\..\dat\data.old del ..\..\dat\data.old
ren ..\..\dat\data~1.bas data.old
:long1ok

if exist ..\..\include\patchlev.h goto long2ok
if exist ..\..\include\patchlevel.h goto long2a
if exist ..\..\include\patchl~1.h goto long2b
goto err_plev

:long2a
echo "Copying ..\..\include\patchlevel.h -> ..\..\include\patchlev.h"
copy ..\..\include\patchlevel.h ..\..\include\patchlev.h
if exist ..\..\include\patchlev.old del ..\..\include\patchlev.old
ren ..\..\include\patchlevel.h patchlev.old
goto long2ok

:long2b
echo "Copying ..\..\include\patchl~1.h -> ..\..\include\patchlev.h"
copy ..\..\include\patchl~1.h ..\..\include\patchlev.h
if exist ..\..\include\patchlev.old del ..\..\include\patchlev.old
ren ..\..\include\patchl~1.h patchlev.old
:long2ok

REM Missing guidebook is not fatal to the build process
if exist ..\..\doc\guidebook.txt goto gbdone
if exist ..\..\doc\guideboo.txt goto long3a
if exist ..\..\doc\guideb~1.txt goto long3b
echo "Warning - There is no NetHack Guidebook (..\..\doc\guideboo.txt)"
echo "          included in your distribution.  Build will proceed anyway."
goto gbdone

:long3a
echo "Copying ..\..\doc\guideboo.txt -> ..\..\doc\guidebook.txt"
copy ..\..\doc\guideboo.txt ..\..\doc\guidebook.txt
goto gbdone

:long3b
echo "Copying ..\..\doc\guideb~1.txt -> ..\..\doc\guidebook.txt"
copy ..\..\doc\guideb~1.txt ..\..\doc\guidebook.txt
goto gbdone

:gbdone

if exist .\nethack.ico goto hasicon
if exist .\nhico.uu uudecode nhico.uu >nul
if NOT exist .\nethack.ico goto err_nouu
:hasicon
echo NetHack icon exists ok.
echo done!
echo.
echo Proceed with the next step documented in Install.nt
goto done
:err_nouu
echo Apparently you have no UUDECODE utility in your path.  You need a UUDECODE
echo utility in order to turn "nhico.uu" into "nethack.ico".
echo Check "Install.nt" for a list of the steps required to build NetHack.
goto done
:err_plev
echo A required file ..\..\include\patchlev.h seems to be missing.
echo Check "Files." in the root directory for your NetHack distribution
echo and make sure that all required files exist.
goto done
:err_data
echo A required file ..\..\dat\data.bas seems to be missing.
echo Check "Files." in the root directory for your NetHack distribution
echo and make sure that all required files exist.
goto done
:err_dir
echo Your directories are not set up properly, please re-read the
echo documentation.
:done
