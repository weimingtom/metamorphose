/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/StringLib.java#1 $
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
	import flash.utils.ByteArray;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	import com.iteye.weimingtom.metamorphose.java.ByteArrayOutputStream;
	
	/**
	 * Contains Lua's string library.
	 * The library can be opened using the {@link #open} method.
	 */
	public final class StringLib extends LuaJavaCallback
	{
		// Each function in the string library corresponds to an instance of
		// this class which is associated (the 'which' member) with an integer
		// which is unique within this class.  They are taken from the following
		// set.
		private static const BYTE:int = 1;
		private static const CHAR:int = 2;
		private static const DUMP:int = 3;
		private static const FIND:int = 4;
		private static const FORMAT:int = 5;
		private static const GFIND:int = 6;
		private static const GMATCH:int = 7;
		private static const GSUB:int = 8;
		private static const LEN:int = 9;
		private static const LOWER:int = 10;
		private static const MATCH:int = 11;
		private static const REP:int = 12;
		private static const REVERSE:int = 13;
		private static const SUB:int = 14;
		private static const UPPER:int = 15;
		
		private static const GMATCH_AUX:int = 16;
		
		private static var GMATCH_AUX_FUN:StringLib = new StringLib(GMATCH_AUX);
		
		/**
		* Which library function this object represents.  This value should
		* be one of the "enums" defined in the class.
		*/
		private var _which:int;
		
		/** Constructs instance, filling in the 'which' member. */
		public function StringLib(which:int) 
		{
			this._which = which;
		}
		  
		/**
		 * Adjusts the output of string.format so that %e and %g use 'e'
		 * instead of 'E' to indicate the exponent.  In other words so that
		 * string.format follows the ISO C (ISO 9899) standard for printf.
		 */
		public function formatISO():void
		{
			FormatItem.E_LOWER = 'e'.charCodeAt();
		}

		/**
		 * Implements all of the functions in the Lua string library.  Do not
		 * call directly.
		 * @param L  the Lua state in which to execute.
		 * @return number of returned parameters, as per convention.
		 */
		override public function luaFunction(L:Lua):int
		{
			switch (this._which)
			{
				case BYTE:
					return byteFunction(L);
			  
				case CHAR:
					return charFunction(L);
			  
				case DUMP:
					return dump(L);
			  
				case FIND:
					return find(L);
			  
				case FORMAT:
					return format(L);
			  
				case GMATCH:
					return gmatch(L);
			  
				case GSUB:
					return gsub(L);
			  
				case LEN:
					return len(L);
			  
				case LOWER:
					return lower(L);
			  
				case MATCH:
					return match(L);
			  
				case REP:
					return rep(L);
			  
				case REVERSE:
					return reverse(L);
			  
				case SUB:
					return sub(L);
			  
				case UPPER:
					return upper(L);
			  
				case GMATCH_AUX:
					return gmatchaux(L);
			}
			return 0;
		}
		
		/**
		 * Opens the string library into the given Lua state.  This registers
		 * the symbols of the string library in a newly created table called
		 * "string".
		 * @param L  The Lua state into which to open.
		 */
		public static function open(L:Lua):void
		{
			var lib:Object = L.__register("string");
			
			r(L, "byte", BYTE);
			r(L, "char", CHAR);
			r(L, "dump", DUMP);
			r(L, "find", FIND);
			r(L, "format", FORMAT);
			r(L, "gfind", GFIND);
			r(L, "gmatch", GMATCH);
			r(L, "gsub", GSUB);
			r(L, "len", LEN);
			r(L, "lower", LOWER);
			r(L, "match", MATCH);
			r(L, "rep", REP);
			r(L, "reverse", REVERSE);
			r(L, "sub", SUB);
			r(L, "upper", UPPER);

			var mt:LuaTable = new LuaTable();
			L.setMetatable("", mt);     // set string metatable
			L.setField(mt, "__index", lib);
		}
		
		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:StringLib = new StringLib(which);
			var lib:Object = L.getGlobal("string");
			L.setField(lib, name, f);
		}
			
		/** Implements string.byte.  Name mangled to avoid keyword. */
		private static function byteFunction(L:Lua):int
		{
			var s:String = L.checkString(1);
			var posi:int = posrelat(L.optInt(2, 1), s);
			var pose:int = posrelat(L.optInt(3, posi), s);
			if (posi <= 0)
			{
				posi = 1;
			}
			if (pose > s.length)
			{
				pose = s.length;
			}
			if (posi > pose)
			{
				return 0; // empty interval; return no values
			}
			var n:int = pose - posi + 1;
			for (var i:int = 0; i < n; ++i)
			{
				L.pushNumber(s.charCodeAt(posi+i-1));
			}
			return n;
		}
			
		/** Implements string.char.  Name mangled to avoid keyword. */
		private static function charFunction(L:Lua):int
		{
			var n:int = L.getTop(); // number of arguments
			var b:StringBuffer = new StringBuffer();
			for (var i:int = 1; i <= n; ++i)
			{
				var c:int = L.checkInt(i);
				L.argCheck(c as uint == c, i, "invalid value");
				b.append(c as uint);
			}
			L.pushString(b.toString());
			return 1;
		}

		/** Implements string.dump. */
		private static function dump(L:Lua):int
		{
			L.checkType(1, Lua.TFUNCTION);
			L.setTop(1);
			try
			{
				var s:ByteArrayOutputStream = new ByteArrayOutputStream();
				Lua.dump(L.value(1), s);
				var a:ByteArray/*byte[]*/ = s.toByteArray();
				s = null;
				var b:StringBuffer = new StringBuffer();
				for (var i:int=0; i<a.length; ++i)
				{
					b.append((uint)(a[i]&0xff));
				}
				L.pushString(b.toString());
				return 1;
			}
			catch (e_:Error)
			{
				trace(e_.getStackTrace());
				L.error("unabe to dump given function");
			}
			// NOTREACHED
			return 0;
		}

		/** Helper for find and match.  Equivalent to str_find_aux. */
		private static function findAux(L:Lua, isFind:Boolean):int
		{
			var s:String = L.checkString(1);
			var p:String = L.checkString(2);
			var l1:int = s.length;
			var l2:int = p.length;
			var init:int = posrelat(L.optInt(3, 1), s) - 1;
			if (init < 0)
			{
				init = 0;
			}
			else if (init > l1)
			{
				init = l1;
			}
			if (isFind && (L.toBoolean(L.value(4)) ||   // explicit request
				strpbrk(p, MatchState.SPECIALS) < 0)) // or no special characters?
			{   // do a plain search
				var off:int = lmemfind(s.substring(init), l1-init, p, l2);
				if (off >= 0)
				{
					L.pushNumber(init+off+1);
					L.pushNumber(init+off+l2);
					return 2;
				}
			}
			else
			{
				var ms:MatchState = new MatchState(L, s, l1);
				var anchor:Boolean = p.charAt(0) == '^';
				var si:int = init;
				do
				{
					ms.level = 0;
					var res:int = ms.match(si, p, anchor ? 1 : 0);
					if (res >= 0)
					{
						if (isFind)
						{
							L.pushNumber(si + 1);       // start
							L.pushNumber(res);          // end
							return ms.push_captures(-1, -1) + 2;
						}     // else
						return ms.push_captures(si, res);
					}
				} while (si++ < ms.end && !anchor);
			}
			L.pushNil();        // not found
			return 1;
		}
			
		/** Implements string.find. */
		private static function find(L:Lua):int
		{
			return findAux(L, true);
		}

		/** Implement string.match.  Operates slightly differently from the
		 * PUC-Rio code because instead of storing the iteration state as
		 * upvalues of the C closure the iteration state is stored in an
		 * Object[3] and kept on the stack.
		 */
		private static function gmatch(L:Lua):int
		{
			var state:Array = new Array(3); //Object[]
			state[0] = L.checkString(1);
			state[1] = L.checkString(2);
			state[2] = new int(0);
			L.pushObject(GMATCH_AUX_FUN);
			L.pushObject(state);
			return 2;
		}
			
		/**
		 * Expects the iteration state, an Object[3] (see {@link
		 * #gmatch}), to be first on the stack.
		 */
		private static function gmatchaux(L:Lua):int
		{
			var state:Array = L.value(1) as Array; //Object[] 
			var s:String = state[0] as String;
			var p:String = state[1] as String;
			var i:int = state[2] as int;
			var ms:MatchState = new MatchState(L, s, s.length);
			for ( ; i <= ms.end ; ++i)
			{
				ms.level = 0;
				var e:int = ms.match(i, p, 0);
				if (e >= 0)
				{
					var newstart:int = e;
					if (e == i)     // empty match?
						++newstart;   // go at least one position
					state[2] = new int(newstart);
					return ms.push_captures(i, e);
				}
			}
			return 0;   // not found.
		}

		/** Implements string.gsub. */
		private static function gsub(L:Lua):int
		{
			var s:String = L.checkString(1);
			var sl:int = s.length;
			var p:String = L.checkString(2);
			var maxn:int = L.optInt(4, sl+1);
			var anchor:Boolean = false;
			if (p.length > 0)
			{
				anchor = p.charAt(0) == '^';
			}
			if (anchor)
				p = p.substring(1);
			var ms:MatchState = new MatchState(L, s, sl);
			var b:StringBuffer = new StringBuffer();
			
			var n:int = 0;
			var si:int = 0;
			while (n < maxn)
			{
				ms.level = 0;
				var e:int = ms.match(si, p, 0);
				if (e >= 0)
				{
					++n;
					ms.addvalue(b, si, e);
				}
				if (e >= 0 && e > si)     // non empty match?
					si = e; // skip it
				else if (si < ms.end)
					b.append(s.charCodeAt(si++));
				else
					break;
				if (anchor)
					break;
			}
			b.appendString(s.substring(si));
			L.pushString(b.toString());
			L.pushNumber(n);    // number of substitutions
			return 2;
		}

		public static function addquoted(L:Lua, b:StringBuffer, arg:int):void
		{
			var s:String = L.checkString(arg);
			var l:int = s.length;
			b.append('"'.charCodeAt());
			for (var i:int = 0; i < l; ++i)
			{
				switch (s.charAt(i))
				{
					case '"': case '\\': case '\n':
						b.append('\\'.charCodeAt());
						b.append(s.charCodeAt(i));
						break;

					case '\r':
						b.appendString("\\r");
						break;
				
					case '\0':
						b.appendString("\\u0000"/*"\\000"*/);
						break;

					default:
						b.append(s.charCodeAt(i));
						break;
				}
			}
			b.append('"'.charCodeAt());
		}

		public static function format(L:Lua):int
		{
			var arg:int = 1;
			var strfrmt:String = L.checkString(1);
			var sfl:int = strfrmt.length;
			var b:StringBuffer = new StringBuffer();
			var i:int = 0;
			while (i < sfl)
			{
				if (strfrmt.charCodeAt(i) != MatchState.L_ESC)
				{
					b.append(strfrmt.charCodeAt(i++));
				}
				else if (strfrmt.charCodeAt(++i) == MatchState.L_ESC)
				{
					b.append(strfrmt.charCodeAt(i++));
				}
				else      // format item
				{
					++arg;
					var item:FormatItem = new FormatItem(L, strfrmt.substring(i));
					i += item.length;
					switch (String.fromCharCode(item.type))
					{
						case 'c':
							item.formatChar(b, L.checkNumber(arg) as uint);
							break;
					
						case 'd': case 'i':
						case 'o': case 'u': case 'x': case 'X':
							// :todo: should be unsigned conversions cope better with
							// negative number?
							item.formatInteger(b, L.checkNumber(arg) as int);
							break;

						case 'e': case 'E': case 'f':
						case 'g': case 'G':
							item.formatFloat(b, L.checkNumber(arg));
							break;

						case 'q':
							addquoted(L, b, arg);
							break;

						case 's':
							item.formatString(b, L.checkString(arg));
							break;

						default:
							return L.error("invalid option to 'format'");
					}
				}
			}
			L.pushString(b.toString());
			return 1;
		}
			
		/** Implements string.len. */
		private static function len(L:Lua):int
		{
			var s:String = L.checkString(1);
			L.pushNumber(s.length);
			return 1;
		}

		/** Implements string.lower. */
		private static function lower(L:Lua):int
		{
			var s:String = L.checkString(1);
			L.pushString(s.toLowerCase());
			return 1;
		}

		/** Implements string.match. */
		private static function match(L:Lua):int
		{
			return findAux(L, false);
		}

		/** Implements string.rep. */
		private static function rep(L:Lua):int
		{
			var s:String = L.checkString(1);
			var n:int = L.checkInt(2);
			var b:StringBuffer = new StringBuffer();
			for (var i:int = 0; i < n; ++i)
			{
				b.appendString(s);
			}
			L.pushString(b.toString());
			return 1;
		}

		/** Implements string.reverse. */
		private static function reverse(L:Lua):int
		{
			var s:String = L.checkString(1);
			var b:StringBuffer = new StringBuffer();
			var l:int  = s.length;
			while (--l >= 0)
			{
				b.append(s.charCodeAt(l));
			}
			L.pushString(b.toString());
			return 1;
		}
			
		/** Helper for {@link #sub} and friends. */
		private static function posrelat(pos:int, s:String):int
		{
			if (pos >= 0)
			{
				return pos;
			}
			var len:int = s.length;
			return len+pos+1;
		}
			
		/** Implements string.sub. */
		private static function sub(L:Lua):int
		{
			var s:String = L.checkString(1);
			var start:int = posrelat(L.checkInt(2), s);
			var end:int = posrelat(L.optInt(3, -1), s);
			if (start < 1)
			{
				start = 1;
			}
			if (end > s.length)
			{
				end = s.length;
			}
			if (start <= end)
			{
				L.pushString(s.substring(start-1, end));
			}
			else
			{
				L.pushLiteral("");
			}
			return 1;
		}
		
		/** Implements string.upper. */
		private static function upper(L:Lua):int
		{
			var s:String = L.checkString(1);
			L.pushString(s.toUpperCase());
			return 1;
		}
			
		/**
		 * @return  character index of start of match (-1 if no match).
		 */
		private static function lmemfind(s1:String, l1:int, s2:String, l2:int):int
		{
			if (l2 == 0)
			{
				return 0; // empty strings are everywhere
			}
			else if (l2 > l1)
			{
				return -1;        // avoids a negative l1
			}
			return s1.indexOf(s2);
		}
			
		/**
		 * Just like C's strpbrk.
		 * @return an index into <var>s</var> or -1 for no match.
		 */
		private static function strpbrk(s:String, _set:String):int
		{
			var l:int = _set.length;
			for (var i:int = 0; i < l; ++i)
			{
				var idx:int = s.indexOf(_set.charAt(i));
				if (idx >= 0)
					return idx;
			}
			return -1;
		}
	}
}