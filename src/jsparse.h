/*
 * jsparse.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSPARSE_H_
#define JSPARSE_H_

#include "jsvar.h"
#include "jslex.h"


typedef struct {
  JsVarRef  root;   /// root of symbol table

  //CScriptLex *l;             /// current lexer
  //std::vector<CScriptVar*> scopes; /// stack of scopes when parsing

  JsVarRef zeroInt;
  JsVarRef oneInt;
  JsVarRef stringClass; /// Built in string class
  JsVarRef objectClass; /// Built in object class
  JsVarRef arrayClass; /// Built in array class
} JsParse;

void jspInit(JsParse *parse);
void jspKill(JsParse *parse);

#if 0
~CTinyJS();

    void execute(const std::string &code);
    /** Evaluate the given code and return a link to a javascript object,
     * useful for (dangerous) JSON parsing. If nothing to return, will return
     * 'undefined' variable type. CScriptVarLink is returned as this will
     * automatically unref the result as it goes out of scope. If you want to
     * keep it, you must use ref() and unref() */
    CScriptVarLink evaluateComplex(const std::string &code);
    /** Evaluate the given code and return a string. If nothing to return, will return
     * 'undefined' */
    std::string evaluate(const std::string &code);

    /// add a native function to be called from TinyJS
    /** example:
       \code
           void scRandInt(CScriptVar *c, void *userdata) { ... }
           tinyJS->addNative("function randInt(min, max)", scRandInt, 0);
       \endcode

       or

       \code
           void scSubstring(CScriptVar *c, void *userdata) { ... }
           tinyJS->addNative("function String.substring(lo, hi)", scSubstring, 0);
       \endcode
    */
    void addNative(const std::string &funcDesc, JSCallback ptr, void *userdata);

    /// Get the given variable specified by a path (var1.var2.etc), or return 0
    CScriptVar *getScriptVariable(const std::string &path);
    /// Get the value of the given variable, or return 0
    const std::string *getVariable(const std::string &path);
    /// set the value of the given variable, return trur if it exists and gets set
    bool setVariable(const std::string &path, const std::string &varData);

    /// Send all variables to stdout
    void trace();



    // parsing - in order of precedence
    CScriptVarLink *functionCall(bool &execute, CScriptVarLink *function, CScriptVar *parent);
    CScriptVarLink *factor(bool &execute);
    CScriptVarLink *unary(bool &execute);
    CScriptVarLink *term(bool &execute);
    CScriptVarLink *expression(bool &execute);
    CScriptVarLink *shift(bool &execute);
    CScriptVarLink *condition(bool &execute);
    CScriptVarLink *logic(bool &execute);
    CScriptVarLink *ternary(bool &execute);
    CScriptVarLink *base(bool &execute);
    void block(bool &execute);
    void statement(bool &execute);
    // parsing utility functions
    CScriptVarLink *parseFunctionDefinition();
    void parseFunctionArguments(CScriptVar *funcVar);

    CScriptVarLink *findInScopes(const std::string &childName); ///< Finds a child, looking recursively up the scopes
    /// Look up in any parent classes of the given object
    CScriptVarLink *findInParentClasses(CScriptVar *object, const std::string &name);
#endif
#endif /* JSPARSE_H_ */
