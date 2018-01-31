break jsAssertFail
break jsError
define jsvTrace
  print jsvTrace(execInfo.root, 0)
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

