/*
 * jslex.h
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#ifndef JSLEX_H_
#define JSLEX_H_

#include "jsutils.h"
#include "jsvar.h"

typedef struct
{
    // Actual Lexing related stuff
    char currCh, nextCh;
    int tk; ///< The type of the token that we have
    int tokenStart; ///< Position in the data at the beginning of the token we have here
    int tokenEnd; ///< Position in the data at the last character of the token we have here
    int tokenLastEnd; ///< Position in the data at the last character of the last token
    char token[JSLEX_MAX_TOKEN_LENGTH]; ///< Data contained in the token we have here
    int tokenl; ///< the current length of token

    /* Where we get our data from...
     *
     * This is a bit more tricky than normal because the data comes from JsVars,
     * which only have fixed length strings. If we go past this, we have to go
     * to the next jsVar...
     */
    JsVarRef sourceVarRef; // the actual string var
    int sourceStartPos;
    int sourceEndPos;
    // current position in data
    int currentPos;
    int currentVarPos; // current position in currentVar
    JsVarRef currentVarRef; // current var
    JsVar currentVar; // current JsVar itself

} JsLex;

void jslInit(JsLex *lex, JsVarRef var, int startPos, int endPos);
void jslKill(JsLex *lex);

//void jslMatch(JsLex lex, int expected_tk); ///< Lexical match wotsit
//std::string jslGetTokenStr(JsLex lex, int token); ///< Get the string representation of the given token
//void jslReset(JsLex lex); ///< Reset this lex so we can start again

//std::string jslGetSubString(JsLex lex, int pos); ///< Return a sub-string from the given position up until right now
//Lex *jslGetSubLex(JsLex lex, int lastPosition); ///< Return a sub-lexer from the given position up until right now

//std::string jslGetPosition(JsLex lex, int pos=-1); ///< Return a string representing the position in lines and columns of the character pos given
void jslGetNextCh(JsLex lex);
void jslGetNextToken(JsLex lex); ///< Get the text token from our text string



#endif /* JSLEX_H_ */
