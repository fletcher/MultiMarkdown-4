@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   multimarkdown -b -t latex %1
SHIFT
GOTO Loop
:Continue

