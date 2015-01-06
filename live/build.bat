cd liveMedia
nmake /B -f liveMedia.mak
copy *.lib ..\lib
cd ..\groupsock
nmake /B -f groupsock.mak
copy *.lib ..\lib
cd ..\UsageEnvironment
nmake /B -f UsageEnvironment.mak
copy *.lib ..\lib
cd ..\BasicUsageEnvironment
nmake /B -f BasicUsageEnvironment.mak
copy *.lib ..\lib
cd ..\testProgs
nmake /B -f testProgs.mak
cd ..\mediaServer
nmake /B -f mediaServer.mak
cd ..
