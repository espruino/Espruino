target extended-remote oc1:4242
monitor reset init

#break jsAssertFail
#break jsErrorAt
#break jsError
#define jtrace
#  print jsvTrace(jsvGetRef(execInfo.root), 0)
#end   
#define whereami
#  print jsiConsolePrintTokenLineMarker(execInfo.lex, execInfo.lex->tokenStart)
#end
#define typeof
#  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_NAME_STRING_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_NAME_STRING_MAX
#    printf "JSV_NAME_STRING_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_NAME_STRING_0
#  end
#  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_STRING_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_STRING_MAX
#    printf "JSV_STRING_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_STRING_0
#  end
#  if (($arg0)->flags&JSV_VARTYPEMASK)>=JSV_STRING_EXT_0 && (($arg0)->flags&JSV_VARTYPEMASK)<=JSV_STRING_EXT_MAX
#    printf "JSV_STRING_EXT_%d\n", (($arg0)->flags&JSV_VARTYPEMASK)-JSV_STRING_EXT_0
#  end
#  print (JsVarFlags)(($arg0)->flags&(JSV_VARTYPEMASK))
#end
