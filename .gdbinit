break jsAssertFail
break jsError
define jsvTrace
  print jsvTrace($arg0, 0)
end   
define whereami
 print jslPrintPosition(jsiConsolePrintString, 0, lex->tokenLastStart)
 print jslPrintTokenLineMarker(jsiConsolePrintString, 0, lex->tokenLastStart, 0)
end
define typeof
  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_NAME_STRING_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_NAME_STRING_MAX
    printf "JSV_NAME_STRING_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_NAME_STRING_0
  end
  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_NAME_STRING_INT_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_NAME_STRING_INT_MAX
    printf "JSV_NAME_STRING_INT_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_NAME_STRING_INT_0
  end
  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_STRING_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_STRING_MAX
    printf "JSV_STRING_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_STRING_0
  end
  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_STRING_EXT_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_STRING_EXT_MAX
    printf "JSV_STRING_EXT_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_STRING_EXT_0
  end
  print (JsVarFlags)(($arg0)->flags&(JSV_VARTYPEMASK))
end
define asm
  set  disassemble-next-line on
  show disassemble-next-line
  echo now use stepi 
end
# Watchdog timer off for NRF52 devices
define wdt_off
  p (*(uint32_t*)0x4001050C)=1
end
define execflags
  if execInfo.execute==0
    printf "EXEC_NO\n"
  end
  if execInfo.execute&EXEC_YES
    printf "EXEC_YES\n"
  end
  if execInfo.execute&EXEC_BREAK
    printf "EXEC_BREAK\n"
  end
  if execInfo.execute&EXEC_CONTINUE
    printf "EXEC_CONTINUE\n"
  end
  if execInfo.execute&EXEC_RETURN
    printf "EXEC_RETURN\n"
  end
  if execInfo.execute&EXEC_INTERRUPTED
    printf "EXEC_INTERRUPTED\n"
  end
  if execInfo.execute&EXEC_EXCEPTION
    printf "EXEC_EXCEPTION\n"
  end
  if execInfo.execute&EXEC_ERROR
    printf "EXEC_ERROR\n"
  end
  if execInfo.execute&EXEC_ERROR_LINE_REPORTED
    printf "EXEC_ERROR_LINE_REPORTED\n"
  end
  if execInfo.execute&EXEC_FOR_INIT
    printf "EXEC_FOR_INIT\n"
  end
  if execInfo.execute&EXEC_IN_LOOP
    printf "EXEC_IN_LOOP\n"
  end
  if execInfo.execute&EXEC_IN_SWITCH
    printf "EXEC_IN_SWITCH\n"
  end
  if execInfo.execute&EXEC_CTRL_C
    printf "EXEC_CTRL_C\n"
  end
  if execInfo.execute&EXEC_CTRL_C_WAIT
    printf "EXEC_CTRL_C_WAIT\n"
  end
end

