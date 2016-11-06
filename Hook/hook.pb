Structure SEARCH_DATA
  processId.l
  handle.i
EndStructure

; Determine if hWnd is the main window (has no parent and is visible)
Procedure isMainWindow(hWnd.i)
  ProcedureReturn Bool(GetWindow_(hWnd, #GW_OWNER) = 0 And IsWindowVisible_(hWnd))
EndProcedure

; A callback function for EnumWindows, called for each window in the list. 
; This callback function determines if the found window is the probable main
; window of the process we want.
Procedure.i enumCallback(hWnd.i, lParam.i)
  Protected *sd.SEARCH_DATA = lParam 
  Protected processId.l = 0
  GetWindowThreadProcessId_(hWnd, @processId)
  If(*sd\processId <> processId Or Not isMainWindow(hWnd))
    ProcedureReturn #True 
  EndIf 
  
  *sd\handle = hWnd
  ProcedureReturn #False 
EndProcedure

; A procedure to find the main window of a process
Procedure.i findMainWindow(pid.l)
  Protected sd.SEARCH_DATA
  sd\processId = pid
  sd\handle    = 0
  EnumWindows_(@enumCallback(), @sd)
  ProcedureReturn sd\handle
EndProcedure

; A thread that prints text to the notepad editor every second.
Procedure thrHijacked(lParam)
  Protected length
  
  SetWindowText_(lParam, "Hello World!! ")
  
  Repeat 
    length = GetWindowTextLength_(lParam)
    SetFocus_(lParam)
    
    ; Append text to the end of the notepad editor
    SendMessage_(lParam, #EM_SETSEL, length, length)
    SendMessage_(lParam, #EM_REPLACESEL, 0, "Hijacked!!! ")
    
    Delay(1000)
  Until Not IsWindow_(lParam) 
EndProcedure

; Keep track of a reference to our text filling thread
Global hThread

; AttachProcess is called automatically when the DLL is loaded.
ProcedureDLL AttachProcess(hInstance)
  ; Find the edit window
  Protected currentProcess = GetCurrentProcessId_()
  Protected mainWindow     = findMainWindow(currentProcess)
  Protected editWindow     = FindWindowEx_(mainWindow, #Null, "Edit", 0)
  
  ; Change notepad's window title and start filling the editor with useless text.
  If(IsWindow_(editWindow)) 
    SetWindowText_(mainWindow, "This process is mine now, /hook.dll!")
    hThread = CreateThread(@thrHijacked(), editWindow)
  EndIf 
  
EndProcedure

; When the DLL is unloaded, kill our text writing thread
ProcedureDLL DetachProcess(hInstance)
  If(IsThread(hThread))
    KillThread(hThread)
  EndIf 
EndProcedure
; IDE Options = PureBasic 5.40 LTS (Windows - x64)
; ExecutableFormat = Shared Dll
; CursorPosition = 23
; FirstLine = 27
; Folding = --
; EnableUnicode
; EnableThread
; EnableXP
; Executable = D:\Projects\Visual Studio\Local\BasicInjection\x64\Release\hook.dll