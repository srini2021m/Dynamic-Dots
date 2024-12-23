#!/usr/bin/env python3

import unittest
import subprocess
import os
import tempfile
from unittest import skipIf

from test_common import Valgrind, SimpleTest, skipIfFailed, load_tests


def clean (s):
  """
  Strips leading spaces from each line
  """
  if isinstance(s, list): s = "\n".join(s)
  s = s.strip().split("\n")
  s = [x.strip() for x in s]
  s = "\n".join(s)
  if s and not s.endswith("\n"): s += "\n"
  return s


def parsekeym (s, k, apply=None):
  s = s.split("\n");
  s = [x.strip() for x in s]
  s = [x.split(":",1) for x in s if ":" in x]
  s = [(a.strip(),b.strip()) for a,b in s]
  s = [b for a,b in s if a == k]
  if apply: s = [apply(x) for x in s]
  return s


class DotsTest:
  # We put the superclass inside another class just so that the test discovery
  # system won't find it and try to run the superclass as a test!
  #@skipIfFailed()
  class Case1 (SimpleTest.Case):
    args = """
    """
    expected = """
    """

    expect_fail = False
    prog = "./test_dots1"

    def _check_output (self):
      self.assertEqual(clean(self.expected), clean(self.e))

  class Case2 (Case1):
    prog = "./test_dots2"

  class Mem1 (Valgrind.Case):
    prog = "./test_dots1"

  class Mem2 (Mem1):
    prog = "./test_dots2"



class Part1a (DotsTest.Case1):
  args = """
  showdots
  """

  expected = """
  -- dots --
  ----------
  """


class Part1b (DotsTest.Case1):
  args = """
  srand 10
  keyx 2 sp 0
  showdots
  key q
  """

  expected = """
  -- dots --
  Dot 0   0   0   85  0   24
  Dot 1   0   0   58  1   24
  ----------
  """


class Part1c (DotsTest.Mem1):
  score = 1

  # This test executes the same operations as Part1b, but this time we are
  # checking for memory errors (due to the different superclass).
  args = Part1b.args


class Part2a (DotsTest.Case1):
  # Press space 9 times, then d nine times, then space 9 times,
  # then d nine times
  args = """
  keyx 9 sp 0
  keyx 9 d 0
  keyx 9 sp 0
  numdots
  keyx 9 d 0
  numdots
  """

  expected = """
  num_dots: 9
  num_dots: 0
  """


class Part2b (DotsTest.Mem1):
  score = 1

  args = Part2a.args


class Part3 (DotsTest.Case1):
  score = 0.25

  args = """
  key sp
  showdots
  mousedown 1
  mousemove 42 142 0
  showdots
  """

  expected = """
  -- dots --
  Dot 0   0   0   73  0   24
  ----------
  -- dots --
  Dot 0   42  142 73  0   24
  ----------
  """


class Part4a (DotsTest.Case2):
  args = """
  keyx 50 sp 0
  numdots
  """

  expected = """
  num_dots: 50
  """


class Part4b (DotsTest.Case2):
  args = """
  srand 10
  keyx 20 sp 0
  showdots
  """

  expected = """
  -- dots --
  Dot 0   0   0   85  0   24
  Dot 1   0   0   58  1   24
  Dot 2   0   0   98  2   24
  Dot 3   0   0   65  3   24
  Dot 4   0   0   58  4   24
  Dot 5   0   0   57  5   24
  Dot 6   0   0   35  6   24
  Dot 7   0   0   81  7   24
  Dot 8   0   0   77  8   24
  Dot 9   0   0   77  9   24
  Dot 10  0   0   83  10  24
  Dot 11  0   0   73  11  24
  Dot 12  0   0   88  12  24
  Dot 13  0   0   63  13  24
  Dot 14  0   0   34  14  24
  Dot 15  0   0   51  15  24
  Dot 16  0   0   90  16  24
  Dot 17  0   0   58  17  24
  Dot 18  0   0   81  18  24
  Dot 19  0   0   68  19  24
  ----------
  """


import ctypes
ptrsz = ctypes.sizeof(ctypes.c_void_p())

class Part4c (DotsTest.Case2):
  args = """
  dotssize
  keyx 10 sp 0
  dotssize
  keyx 990 sp 0
  dotssize
  """

  def _check_output (self):
    vals = parsekeym(clean(self.e), "dots size", int)
    self.assertEqual(len(vals), 3, "Unexpected output")
    a,b,c = vals
    self.assertEqual(a, 0, "List memory used before any dots added")

    self.assertAlmostEqual(b,10*ptrsz,delta=20,msg="List size seems wrong")
    self.assertAlmostEqual(c,1000*ptrsz,delta=20,msg="List size seems wrong")


class Part4d (DotsTest.Mem2):
  score = 1

  args = Part4c.args


class Part5a (DotsTest.Case2):
  args = """
  srand 12
  keyx 500 sp 0
  dotssize
  keyx 490 d 0
  dotssize
  """

  def _check_output (self):
    vals = parsekeym(clean(self.e), "dots size", int)
    self.assertEqual(len(vals), 2, "Unexpected output")
    a,b = vals
    self.assertAlmostEqual(a,500*ptrsz,delta=50,msg="List size seems wrong")
    self.assertAlmostEqual(b,10*ptrsz,delta=50,msg="List size seems wrong")


class Part5b (DotsTest.Case2):
  args = """
  srand 12
  keyx 500 sp 0
  keyx 490 d 0
  showdots
  """

  expected = """
  -- dots --
  Dot 0   0   0   90  0   24
  Dot 1   0   0   74  1   24
  Dot 2   0   0   44  2   24
  Dot 3   0   0   48  3   24
  Dot 4   0   0   53  4   24
  Dot 5   0   0   95  5   24
  Dot 6   0   0   39  6   24
  Dot 7   0   0   83  7   24
  Dot 8   0   0   67  8   24
  Dot 9   0   0   44  9   24
  ----------
  """

class Part5c (DotsTest.Mem2):
  score = 0.75

  args = """
  keyx 30 sp 0
  keyx 30 d 0
  """


from random import Random

class Part6a (DotsTest.Case2):
  p6r = Random()
  p6r.seed(99)
  args = []
  args.append( "srand 99" )
  for _ in range(500):
    args.append( f"mousemove {p6r.randint(0,1000)} {p6r.randint(0,1000)} 0" )
    args.append( f"key sp" )
  for _ in range(800):
    args.append( f"mousemove {p6r.randint(0,1000)} {p6r.randint(0,1000)} 0" )
    args.append( f"key x" )
  args.append( "showdots" )
  args = " ".join(args)

  expected = """
  -- dots --
  Dot 0   413 389 32  0   24
  Dot 1   700 717 67  7   24
  Dot 2   346 84  48  21  24
  Dot 3   83  959 35  48  24
  Dot 4   5   390 34  52  24
  Dot 5   884 735 38  66  24
  Dot 6   386 408 32  88  24
  Dot 7   704 738 45  104 24
  Dot 8   130 952 42  120 24
  Dot 9   694 618 55  128 24
  Dot 10  682 731 46  130 24
  Dot 11  409 3   69  148 24
  Dot 12  133 923 34  184 24
  Dot 13  670 751 43  208 24
  Dot 14  281 648 35  211 24
  Dot 15  267 169 32  214 24
  Dot 16  857 449 39  222 24
  Dot 17  982 834 43  235 24
  Dot 18  237 629 32  247 24
  Dot 19  848 991 54  292 24
  Dot 20  536 416 33  305 24
  Dot 21  4   579 31  311 24
  Dot 22  864 996 46  399 24
  Dot 23  572 926 37  404 24
  ----------
  """


class Part6b (DotsTest.Case2):
  args = Part6a.args.replace("showdots", "dotssize")

  def _check_output (self):
    vals = parsekeym(clean(self.e), "dots size", int)
    self.assertEqual(len(vals), 1, "Unexpected output")
    v = vals[0]
    self.assertAlmostEqual(v,23*ptrsz,delta=30,msg="List size seems wrong")


class Part6c (DotsTest.Mem2):
  score = 0.5

  args = Part6a.args


class Part7a (DotsTest.Case2):
  args = """
  key a
  mousemove 200 200 0
  key a
  showdots
  """

  expected = """
  -- dots --
  Dot 0   200 200 76  1   24
  Dot 1   0   0   73  0   24
  ----------
  """


class Part7b (DotsTest.Case2):
  p7r = Random()
  p7r.seed(99)
  args = []
  args.append( "srand 99" )
  for _ in range(50):
    args.append( f"mousemove {p7r.randint(0,400)} {p7r.randint(0,400)} 0" )
    args.append( f"key a" )
  args.append( "showdots" )
  args = " ".join(args)

  expected = """
  -- dots --
  Dot 0   270 346 85  19  24
  Dot 1   319 250 77  9   24
  Dot 2   275 45  84  8   24
  Dot 3   301 110 96  12  24
  Dot 4   273 41  79  26  24
  Dot 5   290 173 94  17  24
  Dot 6   267 207 41  45  24
  Dot 7   279 206 74  49  24
  Dot 8   240 161 48  21  24
  Dot 9   226 131 35  48  24
  Dot 10  349 108 47  14  24
  Dot 11  338 91  70  16  24
  Dot 12  350 358 67  7   24
  Dot 13  369 313 94  11  24
  Dot 14  196 271 63  6   24
  Dot 15  236 265 42  27  24
  Dot 16  226 304 51  33  24
  Dot 17  242 350 55  38  24
  Dot 18  217 323 51  40  24
  Dot 19  389 44  39  4   24
  Dot 20  396 44  81  20  24
  Dot 21  338 90  88  35  24
  Dot 22  327 108 53  47  24
  Dot 23  91  117 99  2   24
  Dot 24  18  56  33  36  24
  Dot 25  11  87  61  37  24
  Dot 26  127 68  93  3   24
  Dot 27  102 28  58  46  24
  Dot 28  44  73  48  41  24
  Dot 29  183 66  40  30  24
  Dot 30  206 21  41  22  24
  Dot 31  236 26  60  29  24
  Dot 32  214 41  87  39  24
  Dot 33  168 69  71  44  24
  Dot 34  102 306 94  1   24
  Dot 35  42  366 46  18  24
  Dot 36  102 215 81  10  24
  Dot 37  77  236 98  15  24
  Dot 38  104 185 77  25  24
  Dot 39  3   217 70  24  24
  Dot 40  2   195 47  43  24
  Dot 41  137 285 50  23  24
  Dot 42  128 373 70  5   24
  Dot 43  101 310 91  28  24
  Dot 44  90  257 98  34  24
  Dot 45  110 366 93  31  24
  Dot 46  158 316 94  42  24
  Dot 47  82  382 67  32  24
  Dot 48  206 194 32  0   24
  Dot 49  191 198 53  13  24
  ----------
  """


class Part7c (DotsTest.Mem2):
  score = 0.5

  args = Part7b.args



if __name__ == "__main__":
  def _skip ():
    from unittest import TestCase
    import sys

    parts = set()
    for an in range(len(sys.argv)-1,-1,-1):
      a = sys.argv[an]
      if a.startswith("--part=") or a.startswith("--parts="):
        a = a.split("=",1)[1].split(",")
        a = [int(x) for x in a]
        parts.update(a)
        del sys.argv[an]
    #print(sys.argv, parts)
    if not parts: return
    remove = []
    for name,v in globals().items():
      if not v: continue
      k = name
      if k.startswith("Part"):
        k = k[4:]
        while k and not k[-1].isdigit():
          k = k[:-1]
        if not k: continue
        k = int(k)
        if k in parts: continue
        if not isinstance(v, type): continue
        if not issubclass(v, TestCase): continue
        remove.append(name)
    for x in remove:
      del globals()[x]
  _skip()
  from test_common import main
  main()
