@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   multimarkdown -b %1
SHIFT
GOTO Loop
:Continue

