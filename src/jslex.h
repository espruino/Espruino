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

typedef struct JsLex
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
    JsVar *currentVar; // current JsVar itself
} JsLex;

void jslInit(JsLex *lex, JsVar *var, int startPos, int endPos);
void jslInitFromLex(JsLex *lex, JsLex *initFrom, int startPos);
void jslKill(JsLex *lex);
void jslReset(JsLex *lex);

bool jslMatch(JsLex *lex, int expected_tk); ///< Match, and return true on success, false on failure
void jslTokenAsString(int token, char *str, size_t len); ///< output the given token as a string - for debugging
void jslGetTokenString(JsLex *lex, char *str, size_t len);
char *jslGetTokenValueAsString(JsLex *lex);

// Only for more 'internal' use
void jslSeek(JsLex *lex, int seekToChar);
void jslGetNextCh(JsLex *lex);
void jslGetNextToken(JsLex *lex); ///< Get the text token from our text string

#endif /* JSLEX_H_ */
