@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   multimarkdown -b -t odf %1
SHIFT
GOTO Loop
:Continue

