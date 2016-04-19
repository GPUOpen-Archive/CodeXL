: %1 = Destination directory for build
: %2 = Project Dir
: %3 = Optional Destination directory for build
: %4 = Optional Destination directory for build

IF NOT EXIST %1\jqPlot mkdir %1\jqPlot
xcopy /Y %2\jqPlot %1\jqPlot

IF [%3] == [] GOTO END
IF NOT EXIST %3\jqPlot mkdir %3\jqPlot
xcopy /Y %2\jqPlot %3\jqPlot

IF [%4] == [] GOTO END
IF NOT EXIST %4\jqPlot mkdir %4\jqPlot
xcopy /Y %2\jqPlot %4\jqPlot

:END

exit 0

