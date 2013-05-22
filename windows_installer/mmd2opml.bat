@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   multimarkdown -b -t opml %1
SHIFT
GOTO Loop
:Continue

