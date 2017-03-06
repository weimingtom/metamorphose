/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/MathLib.java#1 $
 * Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

package com.iteye.weimingtom.metamorphose.lua;

import java.io.PrintStream;

public final class IOLib extends LuaJavaCallback
{
  private static final int WRITE = 1;
  private int which;

  private IOLib(int which)
  {
    this.which = which;
  }

  public int luaFunction(Lua L)
  {
    switch (which)
    {
      case WRITE:
        return write(L);
    }
    return 0;
  }

  public static void open(Lua L)
  {
    /*LuaTable t = */L.register("io");

    r(L, "write", WRITE);
  }

  /** Register a function. */
  private static void r(Lua L, String name, int which)
  {
    IOLib f = new IOLib(which);
    L.setField(L.getGlobal("io"), name, f);
  }
  
  private static int write(Lua L) {
	  return g_write(L, System.out, 1);
  }

  private static final String NUMBER_FMT = ".14g";
  //FIXME:
  private static int g_write(Lua L, PrintStream stream, int arg)
  {
	//FIXME:
	int nargs = L.getTop(); //FIXME:notice here, original code is 'lua_gettop(L) - 1' (something pushed before?)
	int status = 1;
	for (; nargs != 0; arg++) {
		nargs--;
		if (L.type(arg) == Lua.TNUMBER) {
	      if (status != 0) {
	    	  try {
	    		  //stream.print(String.format("%s", L.toNumber(L.value(arg))));
	    		  //@see http://stackoverflow.com/questions/703396/how-to-nicely-format-floating-numbers-to-string-without-unnecessary-decimal-0
	    		  //stream.print(new DecimalFormat("#.##########").format(L.value(arg)));
	    		  //@see Lua#vmToString
	    		  FormatItem f = new FormatItem(null, NUMBER_FMT);
	    		  StringBuffer b = new StringBuffer();
	    		  Double d = (Double)L.toNumber(L.value(arg));
	    		  f.formatFloat(b, (double)d);
	    		  stream.print(b.toString());
	    	  } catch (Throwable e) {
	    		  status = 0;
	    	  }
	      }
	    } else {
	      String s = L.checkString(arg);
	      if (status != 0) {
	    	  try {
	    		  stream.print(s);
	    	  } catch (Throwable e) {
	    		  status = 0;
	    	  }
	      }
	    }
	}
    return pushresult(L, status, null);
  }
  
  public static int errno = 0;//FIXME: not implemented
  private static int pushresult(Lua L, int i, String filename) {
	  int en = IOLib.errno;  /* calls to Lua API may change this value */
	  if (i != 0) {
	    L.pushBoolean(true);
	    return 1;
	  } else {
	    L.pushNil();
	    if (filename != null) {
	    	//FIXME: not implemented
	      L.pushString(String.format("%s: %s", filename, "io error"/*strerror(en)*/));
	    } else {
	    	//FIXME: not implemented
	      L.pushString(String.format("%s", "io error"/*strerror(en)*/));
	    }
	    L.pushNumber(en);
	    return 3;
	  }
	}
}
