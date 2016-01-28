# Copyright (c) 2011, Bernhard Leiner
# Copyright (c) 2013, 2015 Alexander Belchenko
# All rights reserved.
#
# Redistribution and use in source and binary forms,
# with or without modification, are permitted provided
# that the following conditions are met:
#
# * Redistributions of source code must retain
#   the above copyright notice, this list of conditions
#   and the following disclaimer.
# * Redistributions in binary form must reproduce
#   the above copyright notice, this list of conditions
#   and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of the author nor the names
#   of its contributors may be used to endorse
#   or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

'''Compatibility functions for python 2 and 3.

@author     Bernhard Leiner (bleiner AT gmail com)
@author     Alexander Belchenko (alexander belchenko AT gmail com)
@version    2.0
'''

__docformat__ = "javadoc"


import sys

if sys.version_info[0] >= 3:
    # Python 3
    Python = 3

    def asbytes(s):
        if isinstance(s, bytes):
            return s
        return s.encode('latin1')
    def asstr(s):
        if isinstance(s, str):
            return s
        return s.decode('latin1')

    IntTypes = (int,)
    StrType = str
    UnicodeType = str

    range_g = range     # range generator
    def range_l(*args): # range list
        return list(range(*args))

    def dict_keys(dikt):        # dict keys list
        return list(dikt.keys())
    def dict_keys_g(dikt):      # dict keys generator
        return dikt.keys()
    def dict_items_g(dikt):     # dict items generator
        return dikt.items()

    from io import StringIO, BytesIO

else:
    # Python 2
    Python = 2

    asbytes = str
    asstr = str

    IntTypes = (int, long)
    StrType = basestring
    UnicodeType = unicode

    #range_g = xrange    # range generator
    def range_g(*args):
        # we want to use xrange here but on python 2 it does not work with long ints
        try:
            return xrange(*args)
        except OverflowError:
            start = 0
            stop = 0
            step = 1
            n = len(args)
            if n == 1:
                stop = args[0]
            elif n == 2:
                start, stop = args
            elif n == 3:
                start, stop, step = args
            else:
                raise TypeError('wrong number of arguments in range_g call!')
            if step == 0:
                raise ValueError('step cannot be zero')
            if step > 0:
                def up(start, stop, step):
                    while start < stop:
                        yield start
                        start += step
                return up(start, stop, step)
            else:
                def down(start, stop, step):
                    while start > stop:
                        yield start
                        start += step
                return down(start, stop, step)

    range_l = range     # range list

    def dict_keys(dikt):        # dict keys list
        return dikt.keys()
    def dict_keys_g(dikt):      # dict keys generator
        return dikt.keys()
    def dict_items_g(dikt):     # dict items generator
        return dikt.items()

    from cStringIO import StringIO
    BytesIO = StringIO
