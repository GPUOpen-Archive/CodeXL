;
; AutoIt Version: 3.0
; Language:       English
; Platform:       Win9x/NT
; Author:         Doron Ofek
;
; Script Function:
;   Build CodeXL documentation with WordToHelp.
;   This script uses the AutoIt 3.0 scripting automation language to run the WordToHelp application and generate the CodeXL User Guide CHM and HTML outputs.
;   AutoIt can be freely downloaded from http://www.autoitscript.com
ConsoleWrite (  @CRLF & @CRLF & "[AutoIt3] Beginning script execution." & @CRLF)

Local $ExitCode = 0

If FileSetAttrib(".\CodeXL User Guide\*.*", "-R", 1) Then
   ConsoleWrite("[AutoIt3] Successfully removed read-only attribute."  & @CRLF)
Else
   ConsoleWrite("[AutoIt3] Error removing read-only attribute."  & @CRLF)
   $ExitCode = -1
EndIf

Local $WordToHelpCmd = "C:\Program Files (x86)\Softany\WordToHelp\word2help.exe " & Chr(34) & "C:\jenkins\workspace\CodeXL-Doc\CodeXL\Help\CodeXL User Guide\CodeXL User Guide.wfw" & Chr(34) & " /B"
ConsoleWrite ( "[AutoIt3] Executing command: " & $WordToHelpCmd  & @CRLF)

; Run WordToHelp to generate the documentation
Local $WordToHelpPID = Run($WordToHelpCmd)
If $WordToHelpPID = 0 Then
   ConsoleWrite ( "[AutoIt3] Error: failed to Run WordToHelp"  & @CRLF)
 	$ExitCode = -2
Else
   ConsoleWrite ( "[AutoIt3] Succeeded in running WordToHelp"  & @CRLF)
   ; Now wait for the Word2Help to close before continuing
   Local $WordToHelpClose = ProcessWaitClose ($WordToHelpPID, 360)
   If $WordToHelpClose = 0 Then
	  ConsoleWrite ( "[AutoIt3] Error: timeout reached while waiting for WordToHelp to complete the build."  & @CRLF)
	  $ExitCode = -3
	EndIf
EndIf

ConsoleWrite ( "[AutoIt3] Finished. Exit code = " & $ExitCode  & @CRLF  & @CRLF)
Exit($ExitCode)

; Finished!
