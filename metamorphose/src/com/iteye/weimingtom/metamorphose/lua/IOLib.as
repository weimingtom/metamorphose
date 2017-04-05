/*
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

//see jillcode(Java Implementation of Lua Language, Jill):
//	http://code.google.com/p/jillcode/
//这里的代码移植自jillcode(Lua的Java实现，Jill):
//	http://code.google.com/p/jillcode/	
package com.iteye.weimingtom.metamorphose.lua 
{
	import com.iteye.weimingtom.metamorphose.java.PrintStream;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;

	public final class IOLib extends LuaJavaCallback
	{
		private static const WRITE:int = 1;
		private var which:int;

		public function IOLib(which:int) 
		{
			this.which = which;
		}
		
		override public function luaFunction(L:Lua):int
		{
			switch (which)
			{
				case WRITE:
					return write(L);
			}
			return 0;
		}

		public static function open(L:Lua):void
		{
			/*var t:LuaTable = */L.__register("io");

			r(L, "write", WRITE);
		}
		
		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:IOLib = new IOLib(which);
			L.setField(L.getGlobal("io"), name, f);
		}

		private static function write(L:Lua):int
		{
			return g_write(L, SystemUtil.out, 1);
		}

		private static const NUMBER_FMT:String = ".14g";
		//FIXME:
		private static function g_write(L:Lua, stream:PrintStream, arg:int):int
		{
			//FIXME:
			var nargs:int = L.getTop(); //FIXME:notice here, original code is 'lua_gettop(L) - 1' (something pushed before?)
			var status:int = 1;
			for (; nargs != 0; arg++) {
				nargs--;
				if (L.type(arg) == Lua.TNUMBER) {
					if (status != 0) {
						try {
							//stream.print(String.format("%s", L.toNumber(L.value(arg))));
							//@see http://stackoverflow.com/questions/703396/how-to-nicely-format-floating-numbers-to-string-without-unnecessary-decimal-0
							//stream.print(new DecimalFormat("#.##########").format(L.value(arg)));
							//@see Lua#vmToString
							var f:FormatItem = new FormatItem(null, NUMBER_FMT);
							var b:StringBuffer = new StringBuffer();
							var d:Number = L.toNumber(L.value(arg)) as Number;
							f.formatFloat(b, d);
							stream.print(b.toString());
						} catch (e:Error) {
							status = 0;
						}
					}
				} else {
					var s:String = L.checkString(arg);
					if (status != 0) {
						try {
							stream.print(s);
						} catch (e:Error) {
							status = 0;
						}
					}
				}
				
			}
			return pushresult(L, status, null);
		}
		
		private static var errno:int = 0;//FIXME: not implemented
		private static function pushresult(L:Lua, i:int, filename:String):int
		{	
			var en:int = errno;  /* calls to Lua API may change this value */
			if (i != 0) {
				L.pushBoolean(true);
				return 1;
			} else {
				L.pushNil();
				if (filename != null) {
					//FIXME: not implemented
					//L.pushString(String.format("%s: %s", filename, "io error"/*strerror(en)*/));
					L.pushString("" + filename + ": " + "io error"/*strerror(en)*/);
				} else {
					//FIXME: not implemented
					//L.pushString(String.format("%s", "io error"/*strerror(en)*/));
					L.pushString("" + "io error"/*strerror(en)*/);
				}
				L.pushNumber(en);
				return 3;
			}
		}
	}
}