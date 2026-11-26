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
# Checks spelling in comments in C source files - including the text portions of
# /*JSON{ ... } description */ comments (see scripts/common.py for the format):
#
#   * the free-text description after the JSON
#   * "description" fields inside the JSON
#   * the description strings in "params" and "return"
#
# /*TYPESCRIPT ... */ comments are skipped, as they contain code.
#
# Uses 'aspell' (sudo apt install aspell aspell-en) for checking and suggestions.
#
# Things that look like code (```code```, `code`, camelCase, ALLCAPS, words with
# digits/underscores/dots, URLs, HTML tags) are skipped, as are identifiers that
# appear in the C code of the files being checked. Project-specific words can be
# added to scripts/spellcheck_words.txt (one per line, case-insensitive).
#
# Usage:
#   scripts/check_spelling.py                    # check all non-third-party src/libs/targets files
#   scripts/check_spelling.py src/jswrap_array.c # check specific files or directories
#   scripts/check_spelling.py --json-only        # only check /*JSON comments
#   scripts/check_spelling.py --fix              # interactively fix mistakes
#   scripts/check_spelling.py --list-words       # list unknown words by frequency
# ----------------------------------------------------------------------------------------

import argparse
import os
import re
import shutil
import subprocess
import sys
from collections import Counter

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEFAULT_DICT = os.path.join(SCRIPT_DIR, "spellcheck_words.txt")

DEFAULT_DIRS = ["src", "libs", "targets"]
# Third-party code we don't want to check by default
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

# Comments starting with these contain code, not prose, so are skipped entirely
SKIP_COMMENT_PREFIXES = ("/*TYPESCRIPT",)

MIN_WORD_LEN = 3  # ignore 'eg', 'ie', 'ok', etc

# ----------------------------------------------------------------------------------------
# Finding comments
# ----------------------------------------------------------------------------------------

def scan_c(text):
  """ Split C source into comments and code. Returns (comments, code_identifiers)
  where comments is a list of (start, end) offsets of each whole comment
  (including the /* */ or //) """
  comments = []
  code = []
  i = 0
  n = len(text)
  code_start = 0
  while i < n:
    c = text[i]
    if c == '/' and i+1 < n and text[i+1] == '*':
      code.append(text[code_start:i])
      end = text.find("*/", i+2)
      end = n if end < 0 else end+2
      comments.append((i, end))
      i = code_start = end
    elif c == '/' and i+1 < n and text[i+1] == '/':
      code.append(text[code_start:i])
      end = text.find("\n", i)
      if end < 0: end = n
      comments.append((i, end))
      i = code_start = end
    elif c == '"' or c == "'":
      # skip string/char literal (not treated as code identifiers either)
      code.append(text[code_start:i])
      j = i+1
      while j < n and text[j] != c and text[j] != "\n":
        j += 2 if text[j] == "\\" else 1
      i = code_start = min(j+1, n)
    else:
      i += 1
  code.append(text[code_start:])
  identifiers = set(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", "".join(code)))
  return comments, identifiers


class JsonPosParser:
  """ Minimal JSON parser that records the position and path of every string,
  so we know exactly where in the file each string lives """
  def __init__(self, text, base):
    self.t = text
    self.base = base
    self.i = 0
    self.strings = []  # (path, start, end) - start/end are absolute, excluding quotes

  def ws(self):
    while self.i < len(self.t) and self.t[self.i] in " \t\r\n": self.i += 1

  def value(self, path):
    self.ws()
    c = self.t[self.i]
    if c == '{':
      self.i += 1
      self.ws()
      if self.t[self.i] == '}':
        self.i += 1
        return None
      while True:
        self.ws()
        key = self.string(None)
        self.ws()
        if self.t[self.i] != ':': raise ValueError("expected ':'")
        self.i += 1
        self.value(path + (key,))
        self.ws()
        c = self.t[self.i]
        self.i += 1
        if c == '}': return None
        if c != ',': raise ValueError("expected ','")
    elif c == '[':
      self.i += 1
      self.ws()
      if self.t[self.i] == ']':
        self.i += 1
        return None
      idx = 0
      while True:
        self.value(path + (idx,))
        idx += 1
        self.ws()
        c = self.t[self.i]
        self.i += 1
        if c == ']': return None
        if c != ',': raise ValueError("expected ','")
    elif c == '"':
      return self.string(path)
    else:
      m = re.compile(r"-?[0-9.eE+-]+|true|false|null").match(self.t, self.i)
      if not m: raise ValueError("unexpected character")
      self.i = m.end()
      return None

  def string(self, path):
    if self.t[self.i] != '"': raise ValueError("expected string")
    start = self.i+1
    j = start
    while self.t[j] != '"':
      j += 2 if self.t[j] == "\\" else 1
    self.i = j+1
    if path is not None:
      self.strings.append((path, self.base+start, self.base+j))
    return self.t[start:j]


def is_prose_path(path):
  """ Is the JSON string at this path descriptive text we should spell check? """
  if not path: return False
  if path[0] == "description": return True                     # "description" : "..." or [...]
  if path[0] == "params" and len(path) == 3 and path[2] == 2: return True  # ["name","type","desc"]
  if path[0] == "return" and len(path) == 2 and path[1] == 1: return True  # ["type","desc"]
  return False


def get_regions(text, json_only):
  """ Returns (regions, identifiers). regions is a list of (start, end, kind, is_json_string)
  of text that should be spell checked """
  comments, identifiers = scan_c(text)
  regions = []
  for (start, end) in comments:
    if text.startswith(SKIP_COMMENT_PREFIXES, start):
      continue
    if text.startswith("/*JSON", start):
      body_start = start+6
      body_end = end-2 if text.endswith("*/", 0, end) else end
      body = text[body_start:body_end]
      json_end = body.find("\n}")
      if json_end < 0:
        regions.append((body_start, body_end, "JSON comment", False))
        continue
      json_end += 2
      p = JsonPosParser(body[:json_end], body_start)
      try:
        p.value(())
        for (path, s, e) in p.strings:
          if is_prose_path(path):
            regions.append((s, e, "JSON " + ".".join(str(x) for x in path), True))
          elif path and path[0] in ("name", "class"):
            identifiers.add(text[s:e])  # names of functions/classes are fine to use
      except (ValueError, IndexError):
        # fallback - just check anything that looks like a "description"
        for m in re.finditer(r'"description"\s*:\s*"((?:[^"\\]|\\.)*)"', body[:json_end]):
          regions.append((body_start+m.start(1), body_start+m.end(1), "JSON description", True))
      regions.append((body_start+json_end, body_end, "JSON text", False))
    elif not json_only:
      regions.append((start, end, "comment", False))
  return regions, identifiers

# ----------------------------------------------------------------------------------------
# Finding words
# ----------------------------------------------------------------------------------------

# Things to blank out before looking for words (replaced by spaces so offsets stay the same)
MASKS = [
  re.compile(r"```.*?```", re.DOTALL),          # code blocks
  re.compile(r"`[^`\n]*`"),                     # inline code
  re.compile(r"\]\([^)\s]*\)"),                 # markdown link targets
  re.compile(r"\b[a-zA-Z][a-zA-Z0-9+.-]*://\S+"),  # URLs
  re.compile(r"\bwww\.\S+"),
  re.compile(r"\S+@\S+\.\S+"),                  # emails
  re.compile(r"</?[a-zA-Z][^>\n]*>"),           # HTML tags
  re.compile(r"&#?\w+;"),                       # HTML entities
]
JSON_ESCAPE_RE = re.compile(r"\\(u[0-9a-fA-F]{4}|.)")  # escapes in JSON strings

TOKEN_RE = re.compile(r"\S+")
STRIP_CHARS = "([{<\"'*_,.;:!?)]}>~"
SKIP_PREFIXES = "\\@#$%&"
WORD_RE = re.compile(r"[A-Za-z]+(?:'[A-Za-z]+)*$")


def mask_text(s, is_json_string):
  blank = lambda mo: re.sub(r"[^\n]", " ", mo.group(0))
  if is_json_string: s = JSON_ESCAPE_RE.sub(blank, s)
  for m in MASKS:
    s = m.sub(blank, s)
  return s


def looks_like_code(word):
  if len(word) < MIN_WORD_LEN: return True
  if word.isupper(): return True                         # ACRONYM / DEFINE
  if any(c.isupper() for c in word[1:]): return True     # camelCase / PascalCase
  return False


def find_words(text, start, end, is_json_string):
  """ yield (offset, word, compound) for every word that should be checked. If word
  is part of a hyphenated word (eg 'u-blox'), compound is the whole thing, else None """
  masked = mask_text(text[start:end], is_json_string)
  prev = ""
  for tok in TOKEN_RE.finditer(masked):
    t = tok.group(0)
    prev, is_param_name = t, prev[1:] in ("param", "tparam", "def", "ref") and prev[:1] in "\\@"
    if is_param_name: continue          # @param name
    if t[0] in SKIP_PREFIXES: continue  # \param, @return, #define, $var, %d
    # strip surrounding punctuation
    l = 0
    r = len(t)
    while l < r and t[l] in STRIP_CHARS: l += 1
    while r > l and t[r-1] in STRIP_CHARS: r -= 1
    core = t[l:r]
    if not core: continue
    if core.endswith("'s") or core.endswith("'S"): core = core[:-2]
    # split hyphenated words, then each part must be purely alphabetic
    pos = start + tok.start() + l
    parts = core.split("-")
    compound = core if len(parts) > 1 and all(WORD_RE.match(p) for p in parts) else None
    for part in parts:
      if WORD_RE.match(part) and not looks_like_code(part.replace("'", "")):
        yield (pos, part, compound)
      elif part and not WORD_RE.match(part):
        break  # something odd like a path or expression - skip the rest of the token
      pos += len(part)+1

# ----------------------------------------------------------------------------------------
# Spell checking via aspell
# ----------------------------------------------------------------------------------------

class Speller:
  def __init__(self, langs):
    if not shutil.which("aspell"):
      sys.exit("ERROR: 'aspell' not found. Install with: sudo apt install aspell aspell-en")
    self.langs = langs
    self.cache = {}  # word -> None if OK, else list of suggestions

  def check_words(self, words):
    todo = [w for w in set(words) if w not in self.cache]
    if not todo: return
    results = None
    for lang in self.langs:
      r = self._run_aspell(lang, todo)
      if results is None:
        results = r
      else:  # correct in any language = correct
        results = {w: (None if r[w] is None else results[w]) for w in todo}
    self.cache.update(results)

  def _run_aspell(self, lang, words):
    # '^' prefix stops aspell treating the line as a command
    data = "".join("^" + w + "\n" for w in words)
    try:
      out = subprocess.run(["aspell", "-a", "--lang=" + lang, "--encoding=utf-8"],
                           input=data, capture_output=True, text=True, check=True).stdout
    except subprocess.CalledProcessError as e:
      sys.exit("ERROR running aspell for language '%s': %s" % (lang, e.stderr.strip()))
    lines = out.split("\n")[1:]  # skip version banner
    results = {}
    idx = 0
    for w in words:
      bad = None
      while idx < len(lines) and lines[idx] != "":
        line = lines[idx]
        if line.startswith("&"):
          bad = [s.strip() for s in line.split(":", 1)[1].split(",")]
        elif line.startswith("#"):
          bad = []
        idx += 1
      idx += 1  # blank line
      results[w] = bad
    return results

  def suggestions(self, word):
    return self.cache.get(word)

# ----------------------------------------------------------------------------------------
# Dictionary
# ----------------------------------------------------------------------------------------

def load_dictionary(path):
  words = set()
  if os.path.exists(path):
    with open(path, encoding="utf-8") as f:
      for line in f:
        line = line.split("#", 1)[0].strip()
        if line: words.add(line.lower())
  return words


def add_to_dictionary(path, word):
  existing = []
  if os.path.exists(path):
    with open(path, encoding="utf-8") as f:
      existing = f.read().splitlines()
  header = [l for l in existing if l.startswith("#")]
  body = set(l.strip() for l in existing if l.strip() and not l.startswith("#"))
  body.add(word)
  with open(path, "w", encoding="utf-8") as f:
    f.write("\n".join(header + sorted(body, key=lambda s: (s.lower(), s))) + "\n")

# ----------------------------------------------------------------------------------------
# Files
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


def read_file(path):
  with open(path, encoding="utf-8", errors="surrogateescape", newline="") as f:
    return f.read()


def write_file(path, text):
  with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
    f.write(text)

# ----------------------------------------------------------------------------------------
# Main checking
# ----------------------------------------------------------------------------------------

def is_known(word, dictionary, ignored, identifiers_lower=()):
  lw = word.lower()
  return lw in dictionary or word in ignored or lw in identifiers_lower


def find_mistakes(text, speller, dictionary, ignored, json_only):
  """ returns list of (offset, word, kind, compound), sorted by offset """
  regions, identifiers = get_regions(text, json_only)
  identifiers_lower = set(i.lower() for i in identifiers)
  candidates = []
  for (start, end, kind, is_json_string) in regions:
    for (pos, word, compound) in find_words(text, start, end, is_json_string):
      if is_known(word, dictionary, ignored, identifiers_lower): continue
      if compound and is_known(compound, dictionary, ignored, identifiers_lower): continue
      candidates.append((pos, word, kind, compound))
  speller.check_words([c[1] for c in candidates])
  return sorted((c for c in candidates if speller.suggestions(c[1]) is not None),
                key=lambda c: c[0])


def line_info(text, pos):
  line_no = text.count("\n", 0, pos) + 1
  line_start = text.rfind("\n", 0, pos) + 1
  line_end = text.find("\n", pos)
  if line_end < 0: line_end = len(text)
  return line_no, pos - line_start + 1, text[line_start:line_end]


def match_case(original, replacement):
  if original[:1].isupper() and replacement[:1].islower():
    return replacement[0].upper() + replacement[1:]
  return replacement


class Colours:
  def __init__(self, enabled):
    self.bad = "\033[1;31m" if enabled else ""
    self.good = "\033[1;32m" if enabled else ""
    self.dim = "\033[2m" if enabled else ""
    self.bold = "\033[1m" if enabled else ""
    self.reset = "\033[0m" if enabled else ""


class QuitRequest(Exception):
  pass


def fix_file(path, relpath, text, mistakes, speller, args, dictionary, ignored, previous, col):
  """ Interactively fix mistakes in one file. Returns new text """
  shift = 0
  for (pos, word, kind, compound) in mistakes:
    if is_known(word, dictionary, ignored): continue
    if compound and is_known(compound, dictionary, ignored): continue
    pos += shift
    if text[pos:pos+len(word)] != word: continue  # shouldn't happen, but be safe
    suggestions = list(speller.suggestions(word) or [])
    if word in previous:  # what we picked last time goes first
      prev = match_case(word, previous[word])
      suggestions = [prev] + [s for s in suggestions if s != prev]
    suggestions = suggestions[:9]

    line_no, column, line = line_info(text, pos)
    c0 = column-1
    print()
    print("%s%s:%d:%d%s %s(%s)%s" % (col.bold, relpath, line_no, column, col.reset, col.dim, kind, col.reset))
    print("  " + line[:c0] + col.bad + word + col.reset + line[c0+len(word):])
    if suggestions:
      print("  " + "  ".join("%s%d)%s %s" % (col.good, i+1, col.reset, s) for i, s in enumerate(suggestions)))
    else:
      print("  (no suggestions)")
    print("  %s[1-9] replace, r) type replacement, Enter) skip, i) ignore word, a) add to dictionary, q) save & quit%s" % (col.dim, col.reset))
    if compound:
      print("  %sI) ignore '%s', A) add '%s' to dictionary%s" % (col.dim, compound, compound, col.reset))

    while True:
      try:
        choice = input("  > ").strip()
      except EOFError:
        raise QuitRequest(text)
      replacement = None
      if choice == "":
        break
      elif choice.isdigit() and 1 <= int(choice) <= len(suggestions):
        replacement = suggestions[int(choice)-1]
      elif choice == "r":
        try:
          replacement = input("  replace '%s' with: " % word)
        except EOFError:
          raise QuitRequest(text)
        if not replacement: continue
      elif choice in ("i", "I") and (choice == "i" or compound):
        ignored.add(word if choice == "i" else compound)
        break
      elif choice in ("a", "A") and (choice == "a" or compound):
        w = word if choice == "a" else compound
        add_to_dictionary(args.dictionary, w)
        dictionary.add(w.lower())
        print("  added '%s' to %s" % (w, os.path.relpath(args.dictionary)))
        break
      elif choice == "q":
        raise QuitRequest(text)
      else:
        print("  ?")
        continue
      text = text[:pos] + replacement + text[pos+len(word):]
      shift += len(replacement) - len(word)
      previous[word] = replacement
      break
  return text


def main():
  parser = argparse.ArgumentParser(description="Spell check comments (including /*JSON{...} descriptions) in Espruino C sources")
  parser.add_argument("paths", nargs="*", help="files/directories to check (default: src, libs, targets excluding third-party code)")
  parser.add_argument("--fix", action="store_true", help="interactively fix each mistake")
  parser.add_argument("--json-only", action="store_true", help="only check /*JSON{...} comments")
  parser.add_argument("--lang", default="en_GB,en_US", help="comma-separated aspell languages - a word is OK if valid in any (default: en_GB,en_US)")
  parser.add_argument("--dictionary", default=DEFAULT_DICT, help="project word list (default: scripts/spellcheck_words.txt)")
  parser.add_argument("--list-words", action="store_true", help="just list unknown words with their frequency (handy for building the dictionary)")
  parser.add_argument("--no-colour", action="store_true", help="disable coloured output")
  args = parser.parse_args()

  files = expand_paths(args.paths) if args.paths else default_files()
  speller = Speller([l.strip() for l in args.lang.split(",") if l.strip()])
  dictionary = load_dictionary(args.dictionary)
  ignored = set()   # words ignored for this session (--fix)
  previous = {}     # word -> replacement chosen previously (--fix)
  col = Colours(sys.stdout.isatty() and not args.no_colour)
  counts = Counter()
  total = 0
  changed_files = 0

  try:
    for path in files:
      try:
        text = read_file(path)
      except OSError as e:
        print("WARNING: can't read %s: %s" % (path, e), file=sys.stderr)
        continue
      relpath = os.path.relpath(path)
      mistakes = find_mistakes(text, speller, dictionary, ignored, args.json_only)
      if not mistakes: continue
      total += len(mistakes)
      if args.list_words:
        counts.update(m[1] for m in mistakes)
      elif args.fix:
        try:
          new_text = fix_file(path, relpath, text, mistakes, speller, args, dictionary, ignored, previous, col)
        except QuitRequest as q:
          new_text = q.args[0] if q.args else None
          if new_text is not None and new_text != text:
            write_file(path, new_text)
            print("Saved %s" % relpath)
          raise
        if new_text != text:
          write_file(path, new_text)
          changed_files += 1
          print("Saved %s" % relpath)
      else:
        for (pos, word, kind, compound) in mistakes:
          line_no, column, _ = line_info(text, pos)
          sugg = speller.suggestions(word)[:5]
          print("%s:%d:%d: %s%s%s -> %s" % (relpath, line_no, column, col.bad, word, col.reset,
                                            ", ".join(sugg) if sugg else "(no suggestions)"))
  except QuitRequest:
    print("Quit.")
    return 0
  except KeyboardInterrupt:
    print("\nInterrupted - unsaved changes to the current file were discarded.")
    return 1

  if args.list_words:
    for word, n in counts.most_common():
      print("%5d %s" % (n, word))
  elif args.fix:
    print("\nDone. %d file(s) changed." % changed_files)
  else:
    print("%d possible spelling mistake(s) in %d file(s) checked." % (total, len(files)), file=sys.stderr)
  return 1 if total and not args.fix else 0


if __name__ == "__main__":
  sys.exit(main())
