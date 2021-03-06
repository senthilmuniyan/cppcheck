/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// You should use ++ and -- as prefix whenever possible as these are more
// efficient than postfix operators
//---------------------------------------------------------------------------

#include "checkpostfixoperator.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckPostfixOperator instance;
}

void CheckPostfixOperator::postfixOperator()
{
    if (!_settings->isEnabled("performance"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->type() == Token::eIncDecOp) {
                bool result = false;
                if (Token::Match(tok->tokAt(-2), ";|{|}") && Token::Match(tok->next(), ";|)|,")) {
                    result = true;
                } else if (tok->strAt(-2) == ",") {
                    int ii(1);
                    while (tok->strAt(ii) != ")" && tok->tokAt(ii) != 0) {
                        if (tok->strAt(ii) == ";") {
                            result = true;
                            break;
                        }
                        ++ii;
                    }
                } else if (tok->strAt(-2) == "<<" && tok->strAt(1) == "<<") {
                    result = true;
                }

                if (result && tok->previous()->varId()) {
                    const Variable *var = tok->previous()->variable();
                    if (!var || var->isPointer() || var->isArray() || var->isReference())
                        continue;

                    const Token *decltok = var->nameToken();

                    if (Token::Match(decltok->previous(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator")) {
                        // the variable is an iterator
                        postfixOperatorError(tok);
                    } else if (var->type()) {
                        // the variable is an instance of class
                        postfixOperatorError(tok);
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------


void CheckPostfixOperator::postfixOperatorError(const Token *tok)
{
    reportError(tok, Severity::performance, "postfixOperator",
                "Prefer prefix ++/-- operators for non-primitive types.\n"
                "Prefix ++/-- operators should be preferred for non-primitive types. "
                "Pre-increment/decrement can be more efficient than "
                "post-increment/decrement. Post-increment/decrement usually "
                "involves keeping a copy of the previous value around and "
                "adds a little extra code.");
}
