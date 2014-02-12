@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   multimarkdown -b -t rtf %1
SHIFT
GOTO Loop
:Continue

