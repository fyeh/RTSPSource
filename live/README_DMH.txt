
    Open the ‘win32config’ file and change the TOOLS32=... variable to your VS2008 install directory. For me, it’s TOOLS32=C:\Program Files\Microsoft Visual Studio 9.0\VC
    In ‘win32config’, modify the LINK_OPTS_0=... line from msvcirt.lib to msvcrt.lib. This fixes the link error:
    LINK : fatal error LNK1181: cannot open input file 'msvcirt.lib'
    Open the Visual Studio command prompt.
    From the ‘live’ source directory, run genWindowsMakefiles
    Now you’re ready to build. Simply run the following commands:

    cd liveMedia
    nmake /B -f liveMedia.mak
    cd ..\groupsock
    nmake /B -f groupsock.mak
    cd ..\UsageEnvironment
    nmake /B -f UsageEnvironment.mak
    cd ..\BasicUsageEnvironment
    nmake /B -f BasicUsageEnvironment.mak
    cd ..\testProgs
    nmake /B -f testProgs.mak
    cd ..\mediaServer
    nmake /B -f mediaServer.mak

