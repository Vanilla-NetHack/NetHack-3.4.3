echo off
REM     SCCS Id: @(#)setup.bat   93/01/18
REM     Copyright (c) NetHack PC Development Team 1990, 1991, 1992, 1993

REM     NetHack may be freely redistributed.  See license for details.

REM setup batch file for msdos, see Install.dos for details.

if not %1.==. goto ok_parm
goto err_set

:ok_parm
echo Checking to see if directories are set up properly ...
if not exist ..\..\include\hack.h  goto err_dir
if not exist ..\..\src\hack.c      goto err_dir
if not exist ..\..\dat\wizard.des  goto err_dir
if not exist ..\..\util\makedefs.c goto err_dir
if not exist ..\share\lev_yacc.c   goto err_dir
echo Directories OK.

if %1 == MSC goto ok_msc
if %1 == msc goto ok_msc
if %1 == GCC goto ok_gcc
if %1 == gcc goto ok_gcc
goto err_set

:ok_gcc
echo Symbolic links, msdos style
echo "MakeGCC.src -> ..\..\src\makefile"
copy makegcc.src ..\..\src\makefile
echo "MakeGCC.utl -> ..\..\util\makefile"
copy makegcc.utl ..\..\util\makefile
echo "Makefile.dat -> ..\..\dat\makefile"
copy makefile.dat ..\..\dat\makefile
goto done

:ok_msc
echo Setting Environment variables for Compiler.
set  CL= /AL /G2 /Oo /Gs /Gt18 /Zp1 /W0 /I..\include /nologo /DMOVERLAY
echo Copying Makefiles.
echo "MakeMSC.src -> ..\..\src\makefile"
copy makemsc.src ..\..\src\makefile
echo "MakeMSC.utl -> ..\..\util\makefile"
copy makemsc.utl ..\..\util\makefile
echo "Makefile.dat -> ..\..\dat\makefile"
copy makefile.dat ..\..\dat\makefile
echo Copying in dungeon compiler files.
copy ..\share\*.h    ..\..\include
copy ..\share\lev*.c ..\..\util
copy ..\share\dgn*.c ..\..\util
goto done

:err_set
echo Usage:
echo "%0 <MSC | GCC>"
echo -  Run this batch file with either MSC or GCC as the argument
echo    depending on which compiler you are using.
echo    Currently no support for the Borland set of compilers.
goto end

:err_dir
echo/
echo Your directories are not set up properly, please re-read the
echo Install.dos and README documentation.
goto end

:done
echo Setup Done!
echo Please continue with next step from Install.dos.

:end
