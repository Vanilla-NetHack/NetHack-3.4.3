@echo off
REM  SCCS Id: @(#)setup.bat	 93/02/23		 
REM  Copyright (c) NetHack PC Development Team 1993
REM  NetHack may be freely redistributed.  See license for details. 
REM  Windows NT setup batch file, see Install.nt for details
REM
echo Checking to see if directories are set up properly
if not exist ..\..\include\hack.h goto err_dir
if not exist ..\..\src\hack.c goto err_dir
if not exist ..\..\dat\wizard.des goto err_dir
if not exist ..\..\util\makedefs.c goto err_dir
echo Directories look ok.
if exist ..\..\dat\data.bas goto do_rest
if NOT exist ..\..\dat\data.base goto err_data
copy ..\..\dat\data.base ..\..\dat\data.bas >nul
:do_rest
echo Copying Make files
copy makefile.src ..\..\src\makefile >nul
copy makefile.utl ..\..\util\makefile >nul
copy makefile.dat ..\..\dat\makefile >nul
echo Copied ok.
REM blending with empty. ensures that timestamp will be new
prompt %prompt% >_empty.
echo Copying level and dungeon compiler sources to util.
copy _empty.+..\share\lev_lex.c ..\..\util\lev_lex.c >nul
copy _empty.+..\share\dgn_lex.c ..\..\util\dgn_lex.c >nul
copy _empty.+..\share\lev_yacc.c ..\..\util\lev_yacc.c >nul
copy _empty.+..\share\dgn_yacc.c ..\..\util\dgn_yacc.c >nul
echo Copied ok.
echo Copying level and dungeon compiler header files to include.
copy _empty.+..\share\lev_comp.h ..\..\include\lev_comp.h >nul
copy _empty.+..\share\dgn_comp.h ..\..\include\dgn_comp.h >nul
echo Copied ok.
del _empty. >nul
echo done!
echo.
echo Proceed with the next step documented in Install.nt
goto done
:err_data
echo A required file ..\..\dat\data.bas seems to be missing.
echo Check Files. in the root directory for your NetHack distribution
echo and make sure that all required files exist.
goto done
:err_dir
echo Your directories are not set up properly, please re-read the
echo documentation.
:done
