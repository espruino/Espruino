#!/usr/bin/env python3
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Lints Espruino's C code for patterns that waste flash (and some that leak
# JsVar locks), by parsing each file into a syntax tree with tree-sitter.
#
# Code is parsed *without* preprocessing, so code in every #ifdef branch is
# checked, and statements are never merged across preprocessor directives.
#
# Run with --list-rules to see what is checked. To silence a finding, put
# a comment containing 'code-pattern-ignore' on the same line or the line before.
#
# Usage:
#   scripts/check_code_patterns.py                    # check src, libs, targets (minus third-party)
#   scripts/check_code_patterns.py src/jswrap_array.c # check specific files or directories
#   scripts/check_code_patterns.py --rule unlock-merge --rule set-typed-child
#   scripts/check_code_patterns.py --fix              # rewrite files with the suggested fixes
#   scripts/check_code_patterns.py --summary          # just count findings per rule
#   scripts/check_code_patterns.py --suggest-helpers  # find common patterns that could use a new helper
#
# Run by .github/workflows/lint.yml - which fails if anything is found.
#
# Requires: pip install tree-sitter tree-sitter-c
# ----------------------------------------------------------------------------------------

import argparse
import os
import re
import subprocess
import sys

try:
  import tree_sitter
  import tree_sitter_c
except ImportError:
  print("This tool needs tree-sitter. Install with:\n  pip install tree-sitter tree-sitter-c", file=sys.stderr)
  sys.exit(2)

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)

DEFAULT_DIRS = ["src", "libs", "targets"]
# Third-party code we don't want to check by default (same as check_spelling.py)
EXCLUDE_PREFIXES = [
  "libs/crypto/mbedtls/",
  "libs/tensorflow/tensorflow/",
  "libs/tensorflow/third_party/",
  "libs/banglejs/banglejs3_py32/libs/",
  "libs/filesystem/fat_sd/",
  "libs/network/cc3000/",
  "libs/network/wiznet/",
  "libs/compression/heatshrink/",
  "targets/nrf5x_dfu/sdk",
]
SOURCE_EXTS = (".c", ".h", ".cpp")
IGNORE_MARKER = "code-pattern-ignore"

LANGUAGE = tree_sitter.Language(tree_sitter_c.language())
PARSER = tree_sitter.Parser(LANGUAGE)

# ----------------------------------------------------------------------------------------
# Knowledge about the Espruino API
# ----------------------------------------------------------------------------------------

# jsvObjectSetChildAndUnLock(obj, name, jsvNewFromX(v)) -> helper(obj, name, v)
SET_TYPED_CHILD = {
  "jsvNewFromInteger": "jsvObjectSetIntChild",
  "jsvNewFromFloat":   "jsvObjectSetFloatChild",
  "jsvNewFromBool":    "jsvObjectSetBoolChild",
  "jsvNewFromString":  "jsvObjectSetStringChild",
  "jsvNewFromPin":     "jsvObjectSetPinChild",
}

# jsvGetXAndUnLock(jsvObjectGetChildIfExists(obj, name)) -> helper(obj, name)
GET_TYPED_CHILD = {
  "jsvGetIntegerAndUnLock": "jsvObjectGetIntegerChild",
  "jsvGetFloatAndUnLock":   "jsvObjectGetFloatChild",
  "jsvGetBoolAndUnLock":    "jsvObjectGetBoolChild",
}

# f(... v ...); jsvUnLock(v);  ->  fAndUnLock(... v ...);
#   name : (AndUnLock version, index of the arg it unlocks, number of args, return value can be used)
AND_UNLOCK = {
  "jsvGetInteger":     ("jsvGetIntegerAndUnLock", 0, 1, True),
  "jsvGetFloat":       ("jsvGetFloatAndUnLock", 0, 1, True),
  "jsvGetBool":        ("jsvGetBoolAndUnLock", 0, 1, True),
  "jsvGetLongInteger": ("jsvGetLongIntegerAndUnLock", 0, 1, True),
  "jsvAsString":       ("jsvAsStringAndUnLock", 0, 1, True),
  "jsvAsNumber":       ("jsvAsNumberAndUnLock", 0, 1, True),
  "jsvSkipName":       ("jsvSkipNameAndUnLock", 0, 1, True),
  "jsvSkipOneName":    ("jsvSkipOneNameAndUnLock", 0, 1, True),
  "jsvGetValueOf":     ("jsvGetValueOfAndUnLock", 0, 1, True),
  "jsvAsArrayIndex":   ("jsvAsArrayIndexAndUnLock", 0, 1, True),
  "jsvNewUTF8String":  ("jsvNewUTF8StringAndUnLock", 0, 1, True),
  "jsvIsStringIEqual": ("jsvIsStringIEqualAndUnLock", 0, 2, True),
  "jshGetPinFromVar":  ("jshGetPinFromVarAndUnLock", 0, 1, True),
  "jsfNameFromVar":    ("jsfNameFromVarAndUnLock", 0, 1, True),
  "bleVarToUUID":      ("bleVarToUUIDAndUnLock", 1, 2, True),
  "jsvArrayPush":      ("jsvArrayPushAndUnLock", 1, 2, True),
  "jsvRemoveChild":    ("jsvRemoveChildAndUnLock", 1, 2, True),
  # jsvObjectSetChild returns 'child', jsvObjectSetChildAndUnLock returns void
  "jsvObjectSetChild": ("jsvObjectSetChildAndUnLock", 2, 3, False),
}

# Functions that return a JsVar with a new lock that the caller must unlock.
# Not exhaustive, but these are the common ones.
LOCKED_RETURN = {
  "jsvObjectGetChild", "jsvObjectGetChildIfExists", "jsvObjectGetChildI",
  "jsvAsString", "jsvAsStringAndUnLock", "jsvAsFlatString", "jsvAsNumber", "jsvAsNumberAndUnLock",
  "jsvSkipName", "jsvSkipNameAndUnLock", "jsvSkipOneName", "jsvSkipOneNameAndUnLock",
  "jsvGetArrayItem", "jsvGetArrayIndex", "jsvGetLastArrayItem", "jsvArrayPop", "jsvArrayPopFirst",
  "jsvFindChildFromString", "jsvFindOrAddChildFromString", "jsvGetValueOf", "jsvGetValueOfAndUnLock",
  "jspGetNamedField", "jspGetNamedVariable", "jspGetVarNamedField", "jspExecuteFunction",
  "jspEvaluate", "jspNewObject", "jspNewArray", "jsvLockAgain", "jsvLockAgainSafe",
  "jsvMathsOp", "jsvMathsOpSkipNames", "jsvCopy", "jsvArrayJoin", "jsvGetFirstName",
}
def returns_locked(name):
  return name in LOCKED_RETURN or name.startswith("jsvNew")

# Calls that may execute JavaScript (and so allocate lots of memory)
MAY_RUN_JS = re.compile(r'^(jsp|jswrap_)|Execute|Callback|Evaluate|Emit|jsvAsString|ValueOf')

UNLOCKS = {"jsvUnLock": 1, "jsvUnLock2": 2, "jsvUnLock3": 3, "jsvUnLock4": 4}

# ----------------------------------------------------------------------------------------
# Rules (id -> description)
# ----------------------------------------------------------------------------------------

RULES = {
  "set-typed-child":   "jsvObjectSetChildAndUnLock(o,n,jsvNewFromInteger(x)) -> jsvObjectSetIntChild(o,n,x) (also Float/Bool/String/Pin)",
  "get-typed-child":   "jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(o,n)) -> jsvObjectGetIntegerChild(o,n) (also Float/Bool)",
  "getchild-ifexists": "jsvObjectGetChild(o,n,0) -> jsvObjectGetChildIfExists(o,n)",
  "unlock-setchild":   "jsvUnLock(jsvObjectSetChild(o,n,v)) -> jsvObjectSetChildAndUnLock(o,n,v)",
  "getchild-findchild":"jsvSkipNameAndUnLock(jsvFindChildFromString(o,n)) -> jsvObjectGetChildIfExists(o,n)",
  "unlock-merge":      "jsvUnLock(a);jsvUnLock(b); -> jsvUnLock2(a,b) (up to jsvUnLock4)",
  "unlock-merge-nearby":"jsvUnLock(a);foo();jsvUnLock(b); -> foo();jsvUnLock2(a,b); when foo() doesn't use 'a' (up to 3 statements apart)",
  "unlock-null-check": "if (v) jsvUnLock(v); -> jsvUnLock(v) (jsvUnLock is null-safe)",
  "use-andunlock":     "x=jsvGetInteger(v);jsvUnLock(v); -> x=jsvGetIntegerAndUnLock(v); (and other *AndUnLock functions)",
  "push-string":       "jsvArrayPushAndUnLock(a,jsvNewFromString(s)) -> jsvArrayPushString(a,s)",
  "push-2int":         "jsvArrayPushAndUnLock(a,jsvNewFromInteger(x));jsvArrayPushAndUnLock(a,jsvNewFromInteger(y)); -> jsvArrayPush2Int(a,x,y)",
  "empty-string":      "jsvNewFromString(\"\") -> jsvNewFromEmptyString()",
  "exception-expecting":"jsExceptionHere(t,\"Expecting X, got %t\",v) -> jsExceptionExpecting(t,\"X\",v) (shares the common text)",
  "merge-literals":    "consecutive jsiConsolePrint/jsiConsolePrintf/jsvAppendString/jsvAppendPrintf calls with literal text -> one call",
  "leak":              "a newly locked JsVar is passed to something that doesn't unlock it, or is discarded (a bug, not just size)",
}

# ----------------------------------------------------------------------------------------
# Tree helpers
# ----------------------------------------------------------------------------------------

def unparen(n):
  while n is not None and n.type == "parenthesized_expression" and n.named_child_count == 1:
    n = n.named_children[0]
  return n

def call_name(n):
  n = unparen(n)
  if n is None or n.type != "call_expression": return None
  fn = n.child_by_field_name("function")
  if fn is None or fn.type != "identifier": return None
  return fn.text.decode()

def call_args(n):
  n = unparen(n)
  args = n.child_by_field_name("arguments")
  if args is None: return []
  return [a for a in args.named_children if a.type != "comment"]

def stmt_call(stmt):
  """If stmt is 'f(...);', return the call node"""
  if stmt.type != "expression_statement" or stmt.named_child_count != 1: return None
  c = stmt.named_children[0]
  return c if c.type == "call_expression" else None

def walk(n):
  stack = [n]
  while stack:
    n = stack.pop()
    yield n
    stack.extend(reversed(n.children))

def count_ident(n, name):
  return sum(1 for x in walk(n) if x.type == "identifier" and x.text.decode() == name)

def has_side_effects(n):
  """Could evaluating this expression do something (so its order matters)?"""
  for x in walk(n):
    if x.type in ("call_expression", "assignment_expression", "update_expression", "ERROR"):
      return True
  return False

def is_string_literal(n):
  return n.type in ("string_literal", "concatenated_string")

def literal_parts(n):
  """Inner text of each piece of a string literal (escapes left as-is)"""
  parts = []
  for s in ([n] if n.type == "string_literal" else n.named_children):
    if s.type != "string_literal": return None # eg. a macro in a concatenated string
    t = s.text.decode()
    if not t.startswith('"'): return None # L"..." etc
    parts.append(t[1:-1])
  return parts

def char_to_string_part(n):
  t = n.text.decode()
  if not (t.startswith("'") and t.endswith("'")): return None
  inner = t[1:-1]
  if inner == '"': return '\\"'
  if inner == "\\'": return "'"
  return inner

def join_literal_parts(parts):
  """Join the insides of C string literals into one literal, without letting
  a hex/octal escape run on into the next part"""
  out = ""
  for p in parts:
    if out and p and re.search(r'\\(x[0-9a-fA-F]+|[0-7]{1,3})$', out) and re.match(r'[0-9a-fA-F]', p):
      out += '" "'
    out += p
  return '"' + out + '"'

def text(n):
  return n.text.decode()

def enclosing_function_name(n):
  while n is not None and n.type != "function_definition": n = n.parent
  if n is None: return None
  d = n.child_by_field_name("declarator")
  while d is not None and d.type != "function_declarator":
    d = d.child_by_field_name("declarator")
  if d is None: return None
  ident = d.child_by_field_name("declarator")
  return text(ident) if ident is not None and ident.type == "identifier" else None

# ----------------------------------------------------------------------------------------
# Checking a file
# ----------------------------------------------------------------------------------------

class Finding:
  def __init__(self, rule, node, msg, edits=None):
    self.rule = rule
    self.line = node.start_point[0] + 1
    self.col = node.start_point[1] + 1
    self.msg = msg
    self.edits = edits # list of (start_byte, end_byte, replacement bytes) or None if no auto-fix


class Checker:
  def __init__(self, path, src, rules):
    self.path = path
    self.src = src
    self.rules = rules
    self.findings = []
    self.lines = src.split(b"\n")

  def add(self, rule, node, msg, edits=None):
    if rule not in self.rules: return
    # Don't suggest using a function inside that function's own implementation
    fn = enclosing_function_name(node)
    # (ignoring leading underscores, eg _jsvGetIntegerAndUnLock implements jsvGetIntegerAndUnLock)
    if fn and re.search(r'\b_*' + re.escape(fn.lstrip("_")) + r'\b', msg): return
    line = node.start_point[0]
    for l in (line, line - 1):
      if 0 <= l < len(self.lines) and IGNORE_MARKER.encode() in self.lines[l]:
        return
    if edits is not None:
      edits = [(s, e, r.encode() if isinstance(r, str) else r) for s, e, r in edits]
    self.findings.append(Finding(rule, node, msg, edits))

  def replace(self, node, new):
    return (node.start_byte, node.end_byte, new)

  def delete_stmt(self, stmt, prev):
    """Delete 'stmt' along with the whitespace between it and 'prev'"""
    return (prev.end_byte, stmt.end_byte, b"")

  def indent_of(self, node):
    line = self.lines[node.start_point[0]]
    return line[:len(line) - len(line.lstrip())].decode()

  def check(self):
    tree = PARSER.parse(self.src)
    for n in walk(tree.root_node):
      if n.type == "call_expression":
        self.check_call(n)
      elif n.type == "if_statement":
        self.check_if(n)
      if n.type in ("compound_statement", "case_statement", "preproc_if", "preproc_ifdef",
                    "preproc_else", "preproc_elif", "preproc_elifdef"):
        self.check_sequence(n)
    return self.findings

  # -------------------------------------------------------------- single calls
  def check_call(self, call):
    name = call_name(call)
    if name is None: return
    args = call_args(call)
    fn = call.child_by_field_name("function")

    if name == "jsvObjectSetChildAndUnLock" and len(args) == 3:
      inner = call_name(args[2])
      if inner in SET_TYPED_CHILD and len(call_args(args[2])) == 1:
        helper = SET_TYPED_CHILD[inner]
        v = call_args(args[2])[0]
        self.add("set-typed-child", call, f"use {helper}",
                 [self.replace(call, f"{helper}({text(args[0])}, {text(args[1])}, {text(v)})")])

    if name == "jsvUnLock" and len(args) == 1 and call_name(args[0]) == "jsvObjectSetChild":
      sargs = call_args(args[0])
      if len(sargs) == 3:
        inner = call_name(sargs[2])
        if inner in SET_TYPED_CHILD and len(call_args(sargs[2])) == 1:
          helper = SET_TYPED_CHILD[inner]
          self.add("set-typed-child", call, f"use {helper}",
                   [self.replace(call, f"{helper}({text(sargs[0])}, {text(sargs[1])}, {text(call_args(sargs[2])[0])})")])
        else:
          self.add("unlock-setchild", call, "use jsvObjectSetChildAndUnLock",
                   [self.replace(call, f"jsvObjectSetChildAndUnLock({', '.join(text(a) for a in sargs)})")])

    if name in GET_TYPED_CHILD and len(args) == 1:
      inner = call_name(args[0])
      iargs = call_args(args[0]) if inner else []
      if (inner == "jsvObjectGetChildIfExists" and len(iargs) == 2) or \
         (inner == "jsvObjectGetChild" and len(iargs) == 3 and text(iargs[2]) in ("0", "false")):
        helper = GET_TYPED_CHILD[name]
        self.add("get-typed-child", call, f"use {helper}",
                 [self.replace(call, f"{helper}({text(iargs[0])}, {text(iargs[1])})")])

    if name == "jsvObjectGetChild" and len(args) == 3 and text(args[2]) in ("0", "false"):
      parent_name = call_name(call.parent.parent) if call.parent and call.parent.type == "argument_list" else None
      if parent_name not in GET_TYPED_CHILD: # get-typed-child does better
        self.add("getchild-ifexists", call, "use jsvObjectGetChildIfExists",
                 [self.replace(call, f"jsvObjectGetChildIfExists({text(args[0])}, {text(args[1])})")])

    if name == "jsvSkipNameAndUnLock" and len(args) == 1:
      inner = call_name(args[0])
      helper = {"jsvFindChildFromString": "jsvObjectGetChildIfExists",
                "jsvFindChildFromStringI": "jsvObjectGetChildI"}.get(inner)
      if helper and len(call_args(args[0])) == 2:
        self.add("getchild-findchild", call, f"use {helper}",
                 [self.replace(call, f"{helper}({', '.join(text(a) for a in call_args(args[0]))})")])

    if name == "jsvArrayPushAndUnLock" and len(args) == 2 and \
       call_name(args[1]) == "jsvNewFromString" and len(call_args(args[1])) == 1:
      self.add("push-string", call, "use jsvArrayPushString",
               [self.replace(call, f"jsvArrayPushString({text(args[0])}, {text(call_args(args[1])[0])})")])

    if name == "jsExceptionHere" and len(args) == 3 and args[1].type == "string_literal":
      m = re.fullmatch(r'"Expecting ([^"%\\]*), got %t"', text(args[1]))
      if m:
        self.add("exception-expecting", call, "use jsExceptionExpecting",
                 [self.replace(call, f'jsExceptionExpecting({text(args[0])}, "{m.group(1)}", {text(args[2])})')])

    if name == "jsvNewFromString" and len(args) == 1 and text(args[0]) == '""':
      self.add("empty-string", call, "use jsvNewFromEmptyString()",
               [self.replace(call, "jsvNewFromEmptyString()")])

    # ---- leaks
    if name in AND_UNLOCK:
      andunlock, idx, nargs, ret_ok = AND_UNLOCK[name]
      used = call.parent.type != "expression_statement"
      # if !ret_ok, the function returns the arg - so if the result is used, it can be unlocked later
      if len(args) == nargs and returns_locked(call_name(args[idx]) or "") and (ret_ok or not used):
        self.add("leak", call, f"{call_name(args[idx])}(...) result is never unlocked - use {andunlock}",
                 [self.replace(fn, andunlock)])
    if call.parent.type == "expression_statement" and returns_locked(name) and \
       name not in ("jsvLockAgain", "jsvLockAgainSafe"):
      self.add("leak", call, f"result of {name}(...) is discarded but needs unlocking",
               [self.replace(call, f"jsvUnLock({text(call)})")])

  # -------------------------------------------------------------- if (v) jsvUnLock(v);
  def check_if(self, n):
    if n.child_by_field_name("alternative") is not None: return
    cond = unparen(n.child_by_field_name("condition"))
    body = n.child_by_field_name("consequence")
    if cond is None or body is None: return
    if cond.type == "binary_expression" and cond.child_by_field_name("operator").type == "!=" and \
       text(cond.child_by_field_name("right")) in ("0", "NULL"):
      cond = cond.child_by_field_name("left")
    if cond.type != "identifier": return
    if body.type == "compound_statement":
      stmts = [s for s in body.named_children]
      if len(stmts) != 1: return
      body = stmts[0]
    call = stmt_call(body)
    if call is None or call_name(call) != "jsvUnLock": return
    args = call_args(call)
    if len(args) == 1 and text(args[0]) == text(cond):
      self.add("unlock-null-check", n, "jsvUnLock is null-safe, so the 'if' isn't needed",
               [self.replace(n, f"jsvUnLock({text(cond)});")])

  # -------------------------------------------------------------- statement sequences
  def check_sequence(self, block):
    # All named children - comments break up runs, so we never delete them
    stmts = list(block.named_children)
    # Order matters - when fixes overlap the first one found wins
    self.seq_use_andunlock(stmts)
    self.seq_unlock_merge(stmts)
    self.seq_unlock_merge_nearby(stmts)
    self.seq_push_2int(stmts)
    self.seq_merge_literals(stmts)
    self.seq_temp_var(stmts)

  def seq_runs(self, stmts, key):
    """Split stmts into maximal runs of adjacent stmts where key(stmt) is not None and equal"""
    runs, cur, curkey = [], [], None
    for s in stmts + [None]:
      k = key(s) if s is not None else None
      if k is not None and k == curkey:
        cur.append(s)
        continue
      if len(cur) > 1: runs.append(cur)
      cur, curkey = ([s], k) if k is not None else ([], None)
    return runs

  def seq_unlock_merge(self, stmts):
    def key(s):
      c = stmt_call(s)
      if c is None: return None
      name = call_name(c)
      if name not in UNLOCKS or len(call_args(c)) != UNLOCKS[name]: return None
      if any(has_side_effects(a) for a in call_args(c)): return None
      return "unlock"
    for run in self.seq_runs(stmts, key):
      vars = [text(a) for s in run for a in call_args(stmt_call(s))]
      ncalls = (len(vars) + 3) // 4
      if ncalls >= len(run): continue
      calls = []
      for i in range(0, len(vars), 4):
        chunk = vars[i:i+4]
        calls.append(f"jsvUnLock{len(chunk) if len(chunk) > 1 else ''}({', '.join(chunk)});")
      sep = "\n" + self.indent_of(run[0])
      self.add("unlock-merge", run[0], f"{len(run)} unlock calls can be {ncalls}: {' '.join(calls)}",
               [(run[0].start_byte, run[-1].end_byte, sep.join(calls))])

  def unlock_stmt_args(self, s):
    """If s is jsvUnLockN(...) with simple args, return the args"""
    c = stmt_call(s)
    if c is None: return None
    name = call_name(c)
    if name not in UNLOCKS or len(call_args(c)) != UNLOCKS[name]: return None
    args = call_args(c)
    if any(has_side_effects(a) for a in args): return None
    return args

  def seq_unlock_merge_nearby(self, stmts):
    MAX_GAP = 3
    used = set()
    for i, s in enumerate(stmts):
      a1 = self.unlock_stmt_args(s)
      if a1 is None or i in used: continue
      # base identifiers of what we unlock (eg 'foo' for foo->bar or foo[1])
      names = {text(x) for a in a1 for x in walk(a) if x.type == "identifier"}
      gap = 0
      for j in range(i + 1, min(len(stmts), i + 2 + MAX_GAP)):
        t = stmts[j]
        # A comment may explain why the unlock is where it is (eg. to free memory before
        # running a callback) - so don't move unlocks past comments
        if t.type == "comment": break
        a2 = self.unlock_stmt_args(t)
        if a2 is not None:
          if gap and len(a1) + len(a2) <= 4 and j not in used:
            allargs = [text(a) for a in a1 + a2]
            n = len(allargs)
            prev = s.prev_sibling
            newcall = f"jsvUnLock{n}({', '.join(allargs)})"
            self.add("unlock-merge-nearby", s,
                     f"move this unlock down {gap} statement(s) and merge: {newcall};",
                     [(prev.end_byte if prev is not None else s.start_byte, s.end_byte, b""),
                      self.replace(stmt_call(t), newcall)])
            used.update((i, j))
          break
        # Only move an unlock past simple statements that don't touch what is unlocked
        if t.type not in ("expression_statement", "declaration"): break
        if any(x.type == "identifier" and text(x) in names for x in walk(t)): break
        # ALL_CAPS calls are probably macros (which may return/jump)
        if any(x.type == "call_expression" and (call_name(x) or "X").isupper() for x in walk(t)): break
        # Don't keep vars allocated while running JS code
        if any(x.type == "call_expression" and MAY_RUN_JS.search(call_name(x) or "x") for x in walk(t)): break
        if any(x.type in ("ERROR", "goto_statement", "return_statement") for x in walk(t)): break
        gap += 1

  def seq_use_andunlock(self, stmts):
    for prev, s in zip(stmts, stmts[1:]):
      c = stmt_call(s)
      if c is None or call_name(c) not in UNLOCKS: continue
      uargs = call_args(c)
      if prev.type not in ("expression_statement", "declaration"): continue
      for ui, ua in enumerate(uargs):
        if ua.type != "identifier": continue
        v = text(ua)
        if count_ident(prev, v) != 1: continue
        if prev.type == "declaration" and any(
            text(d.child_by_field_name("declarator") or d) == v for d in prev.named_children if d.type == "init_declarator"):
          continue
        target = self.find_andunlock_call(prev, v)
        if target is None: continue
        tcall, andunlock = target
        rest = [text(a) for j, a in enumerate(uargs) if j != ui]
        edits = [self.replace(tcall.child_by_field_name("function"), andunlock)]
        if rest:
          edits.append(self.replace(c, f"jsvUnLock{len(rest) if len(rest) > 1 else ''}({', '.join(rest)})"))
        else:
          edits.append(self.delete_stmt(s, prev))
        self.add("use-andunlock", tcall, f"use {andunlock}({v}) instead of a separate unlock", edits)
        break

  def find_andunlock_call(self, stmt, v):
    for n in walk(stmt):
      name = call_name(n) if n.type == "call_expression" else None
      if name not in AND_UNLOCK: continue
      andunlock, idx, nargs, ret_ok = AND_UNLOCK[name]
      args = call_args(n)
      if len(args) != nargs or args[idx].type != "identifier" or text(args[idx]) != v: continue
      if not ret_ok and n.parent.type != "expression_statement": continue
      # The call must always be evaluated - not inside && || ?: or a lambda-ish construct
      p, conditional = n.parent, False
      while p is not None and p != stmt:
        if p.type == "conditional_expression" or (p.type == "binary_expression" and
            p.child_by_field_name("operator").type in ("&&", "||")):
          conditional = True
        p = p.parent
      if not conditional:
        return n, andunlock
    return None

  def seq_push_2int(self, stmts):
    def key(s):
      c = stmt_call(s)
      if c is None or call_name(c) != "jsvArrayPushAndUnLock": return None
      a = call_args(c)
      if len(a) != 2 or call_name(a[1]) != "jsvNewFromInteger" or has_side_effects(a[0]): return None
      return text(a[0])
    for run in self.seq_runs(stmts, key):
      arr = key(run[0])
      vals = [text(call_args(call_args(stmt_call(s))[1])[0]) for s in run]
      calls = [f"jsvArrayPush2Int({arr}, {vals[i]}, {vals[i+1]});" for i in range(0, len(vals) - 1, 2)]
      if len(vals) % 2: calls.append(f"jsvArrayPushAndUnLock({arr}, jsvNewFromInteger({vals[-1]}));")
      sep = "\n" + self.indent_of(run[0])
      self.add("push-2int", run[0], f"use jsvArrayPush2Int for {len(vals)} integer pushes",
               [(run[0].start_byte, run[-1].end_byte, sep.join(calls))])

  def seq_merge_literals(self, stmts):
    # (function, args before the text) - first arg in 'target' must match across the run
    MERGEABLE = {"jsiConsolePrint": 0, "jsiConsolePrintString": 0, "jsiConsolePrintf": 0,
                 "jsvAppendString": 1, "jsvAppendPrintf": 1, "jsvAppendCharacter": 1}
    def info(s):
      c = stmt_call(s)
      if c is None: return None
      name = call_name(c)
      if name not in MERGEABLE: return None
      a = call_args(c)
      t = MERGEABLE[name]
      if len(a) <= t: return None
      target = text(a[0]) if t else ""
      if t and has_side_effects(a[0]): return None
      txt = a[t]
      if name == "jsvAppendCharacter":
        if len(a) != 2 or txt.type != "char_literal": return None
        parts = [char_to_string_part(txt)]
        if parts[0] is None: return None
      else:
        if not is_string_literal(txt): return None
        parts = literal_parts(txt)
        if parts is None: return None
      extra = a[t+1:]
      isprintf = name.endswith("Printf")
      if extra and (not isprintf or any(has_side_effects(x) for x in extra)): return None
      family = "console" if name.startswith("jsiConsole") else "append"
      return family, target, isprintf, parts, [text(x) for x in extra]
    def key(s):
      i = info(s)
      return None if i is None else (i[0], i[1])
    for run in self.seq_runs(stmts, key):
      infos = [info(s) for s in run]
      family, target = infos[0][0], infos[0][1]
      anyprintf = any(i[2] for i in infos)
      parts, extra = [], []
      for i in infos:
        p = i[3]
        if anyprintf and not i[2]: # plain text going into a printf - escape '%'
          p = [x.replace("%", "%%") for x in p]
        parts += p
        extra += i[4]
      if family == "console":
        fname = "jsiConsolePrintf" if anyprintf else "jsiConsolePrint"
        call = f"{fname}({join_literal_parts(parts)}{''.join(', ' + e for e in extra)});"
      else:
        fname = "jsvAppendPrintf" if anyprintf else "jsvAppendString"
        call = f"{fname}({target}, {join_literal_parts(parts)}{''.join(', ' + e for e in extra)});"
      self.add("merge-literals", run[0], f"merge {len(run)} calls into one {fname}",
               [(run[0].start_byte, run[-1].end_byte, call)])

  def seq_temp_var(self, stmts):
    """JsVar *t = jsvNewFromX(v); jsvObjectSetChildAndUnLock(o,n,t);  -> jsvObjectSetXChild(o,n,v)
       JsVar *t = jsvNewFromString(s); jsvArrayPushAndUnLock(a,t); -> jsvArrayPushString(a,s)"""
    for prev, s in zip(stmts, stmts[1:]):
      if prev.type != "declaration": continue
      decls = [d for d in prev.named_children if d.type == "init_declarator"]
      if len(decls) != 1 or len(prev.named_children) != 2: continue
      d = decls[0]
      dname = d.child_by_field_name("declarator")
      if dname is None or dname.type != "pointer_declarator": continue
      ident = dname.child_by_field_name("declarator")
      if ident is None or ident.type != "identifier": continue
      v = text(ident)
      init = d.child_by_field_name("value")
      iname = call_name(init)
      c = stmt_call(s)
      if c is None or iname is None: continue
      cname, cargs = call_name(c), call_args(c)
      if cname == "jsvObjectSetChildAndUnLock" and len(cargs) == 3 and iname in SET_TYPED_CHILD:
        helper, rule, keep = SET_TYPED_CHILD[iname], "set-typed-child", cargs[:2]
        if text(cargs[2]) != v: continue
      elif cname == "jsvArrayPushAndUnLock" and len(cargs) == 2 and iname == "jsvNewFromString":
        helper, rule, keep = "jsvArrayPushString", "push-string", cargs[:1]
        if text(cargs[1]) != v: continue
      else:
        continue
      if any(has_side_effects(k) for k in keep): continue
      # 't' must not be used anywhere else in the function
      fn = s.parent
      while fn is not None and fn.type != "function_definition": fn = fn.parent
      if fn is None or count_ident(fn, v) != 2: continue
      ival = call_args(init)[0]
      newcall = f"{helper}({', '.join(text(k) for k in keep)}, {text(ival)});"
      self.add(rule, prev, f"temporary '{v}' can be removed: {newcall}",
               [(prev.start_byte, s.end_byte, newcall)])


def suggest_helpers(files, top):
  """Count common patterns that have no helper function yet"""
  from collections import Counter
  nested, unlock_after = Counter(), Counter()
  consumers = {v[0]: v[1] for v in AND_UNLOCK.values()}
  consumers.update({"jsvObjectSetChildAndUnLock": 2, "jsvArrayPushAndUnLock": 1, "jsvUnLock": 0})
  consumers.update({k: 0 for k in GET_TYPED_CHILD})
  known = {("jsvObjectSetChildAndUnLock", k) for k in SET_TYPED_CHILD} | \
          {(k, "jsvObjectGetChildIfExists") for k in GET_TYPED_CHILD} | \
          {("jsvArrayPushAndUnLock", "jsvNewFromString"), ("jsvUnLock", "jsvObjectSetChild"),
           ("jsvSkipNameAndUnLock", "jsvFindChildFromString")}
  for path in files:
    try:
      with open(path, "rb") as fh: src = fh.read()
    except OSError: continue
    tree = PARSER.parse(src)
    for n in walk(tree.root_node):
      if n.type == "call_expression":
        outer = call_name(n)
        if outer in consumers:
          args = call_args(n)
          idx = consumers[outer]
          if idx < len(args):
            inner = call_name(args[idx])
            if inner and (outer, inner) not in known:
              nested[f"{outer}(..{inner}(..)..)"] += 1
      if n.type == "compound_statement":
        st = list(n.named_children)
        for a, b in zip(st, st[1:]):
          cb = stmt_call(b)
          if cb is None or call_name(cb) != "jsvUnLock": continue
          ua = call_args(cb)
          if len(ua) != 1 or ua[0].type != "identifier": continue
          v = text(ua[0])
          if a.type not in ("expression_statement", "declaration"): continue
          for x in walk(a):
            name = call_name(x) if x.type == "call_expression" else None
            if not name or name in AND_UNLOCK or name in UNLOCKS: continue
            for i, arg in enumerate(call_args(x)):
              if arg.type == "identifier" and text(arg) == v:
                unlock_after[f"{name}(arg {i}); jsvUnLock(arg {i})"] += 1
  print("Nested calls that could become a single helper call:")
  for k, v in nested.most_common(top): print(f"  {v:4}  {k}")
  print("\nCalls followed by unlocking one of their arguments (candidates for an ...AndUnLock function):")
  for k, v in unlock_after.most_common(top): print(f"  {v:4}  {k}")


# ----------------------------------------------------------------------------------------
# Files / fixing
# ----------------------------------------------------------------------------------------

def default_files():
  try:
    out = subprocess.run(["git", "ls-files"] + DEFAULT_DIRS, cwd=ROOT_DIR,
                         capture_output=True, text=True, check=True).stdout
    files = out.split("\n")
  except (subprocess.CalledProcessError, FileNotFoundError):
    files = []
    for d in DEFAULT_DIRS:
      for dirpath, _, names in os.walk(os.path.join(ROOT_DIR, d)):
        files += [os.path.relpath(os.path.join(dirpath, n), ROOT_DIR) for n in names]
  return [os.path.join(ROOT_DIR, f) for f in sorted(files)
          if f.endswith(SOURCE_EXTS) and not any(f.startswith(p) for p in EXCLUDE_PREFIXES)]


def expand_paths(paths):
  files = []
  for p in paths:
    if os.path.isdir(p):
      for dirpath, _, names in os.walk(p):
        files += [os.path.join(dirpath, n) for n in sorted(names) if n.endswith(SOURCE_EXTS)]
    else:
      files.append(p)
  return files


def apply_fixes(src, findings):
  """Apply non-overlapping fixes. Returns (new source, number applied)"""
  accepted = []
  for f in findings:
    if not f.edits: continue
    if any(not (e[1] <= a[0] or e[0] >= a[1]) for e in f.edits for a in accepted): continue
    accepted.extend(f.edits)
  accepted.sort(key=lambda e: e[0], reverse=True)
  for s, e, r in accepted:
    src = src[:s] + r + src[e:]
  return src, len(accepted)


def main():
  ap = argparse.ArgumentParser(description="Find C code patterns in Espruino that waste flash memory")
  ap.add_argument("paths", nargs="*", help="files/directories to check (default: src, libs, targets excluding third-party code)")
  ap.add_argument("--rule", action="append", help="only check this rule (can be given more than once)")
  ap.add_argument("--no-rule", action="append", default=[], help="don't check this rule")
  ap.add_argument("--list-rules", action="store_true", help="list the rules and exit")
  ap.add_argument("--fix", action="store_true", help="apply the suggested fixes in place")
  ap.add_argument("--summary", action="store_true", help="only print the number of findings per rule")
  ap.add_argument("--github", action="store_true",
                  help="also output GitHub Actions annotations so findings show up on pull requests")
  ap.add_argument("--suggest-helpers", action="store_true",
                  help="instead of linting, list common patterns that don't have a helper function yet")
  args = ap.parse_args()

  if args.list_rules:
    for r, d in RULES.items(): print(f"{r:20} {d}")
    return 0
  rules = set(args.rule or RULES.keys()) - set(args.no_rule)
  bad = rules - set(RULES)
  if bad:
    print(f"Unknown rule(s): {', '.join(sorted(bad))}. Use --list-rules", file=sys.stderr)
    return 2

  files = expand_paths(args.paths) if args.paths else default_files()
  if args.suggest_helpers:
    suggest_helpers(files, 25)
    return 0
  counts = {}
  for path in files:
    try:
      with open(path, "rb") as fh: src = fh.read()
    except OSError as e:
      print(f"{path}: {e}", file=sys.stderr)
      continue
    findings = Checker(path, src, rules).check()
    rel = os.path.relpath(path)
    for f in findings:
      counts[f.rule] = counts.get(f.rule, 0) + 1
      fixnote = "" if f.edits else " (no auto-fix)"
      if not args.summary:
        print(f"{rel}:{f.line}:{f.col}: [{f.rule}] {f.msg}{fixnote}")
      if args.github:
        print(f"::error file={rel},line={f.line},col={f.col},title=check_code_patterns: {f.rule}::"
              f"{f.msg}{fixnote} - fix with scripts/check_code_patterns.py --fix, or add a 'code-pattern-ignore' comment")
    if args.fix and findings:
      # Fixes can overlap or enable each other (eg. use-andunlock then set-typed-child) so iterate
      for _ in range(10):
        src, n = apply_fixes(src, findings)
        if not n: break
        findings = Checker(path, src, rules).check()
      with open(path, "wb") as fh: fh.write(src)

  total = sum(counts.values())
  if counts:
    print("\nSummary:" if not args.summary else "Summary:")
    for r in RULES:
      if r in counts: print(f"  {r:20} {counts[r]}")
  print(f"{total} finding(s) in {len(files)} file(s)")
  if args.fix:
    remaining = 0
    for path in files:
      with open(path, "rb") as fh: src = fh.read()
      remaining += len(Checker(path, src, rules).check())
    print(f"After fixing, {remaining} finding(s) remain (those without auto-fixes, or that need manual review)")
  return 1 if total else 0


if __name__ == "__main__":
  sys.exit(main())
