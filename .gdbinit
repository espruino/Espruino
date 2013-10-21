break jsAssertFail
break jsErrorAt
break jsError
define trace
  print jsvTrace(jsvGetRef(jsiGetParser()->root), 0)
end   
define whereami
  print jsiConsolePrintTokenLineMarker(execInfo.lex, execInfo.lex->tokenStart)
end
define typeof
  if ($arg0)->flags&JSV_NAME 
    print JSV_NAME
  end
  print (JsVarFlags)(($arg0)->flags&(JSV_VARTYPEMASK))
end
