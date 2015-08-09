/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/BaseLib.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.Enumeration;
	import com.iteye.weimingtom.metamorphose.java.NumberFormatException;
	import com.iteye.weimingtom.metamorphose.java.PrintStream;
	import com.iteye.weimingtom.metamorphose.java.Reader;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;
	
	/**
	 * Contains Lua's base library.  The base library is generally
	 * considered essential for running any Lua program.  The base library
	 * can be opened using the {@link #open} method.
	 */
	public final class BaseLib extends LuaJavaCallback
	{
		// :todo: consider making the enums contiguous so that the compiler
		// uses the compact and faster form of switch.

		// Each function in the base library corresponds to an instance of
		// this class which is associated (the 'which' member) with an integer
		// which is unique within this class.  They are taken from the following
		// set.
		private static const ASSERT:int = 1;
		private static const COLLECTGARBAGE:int = 2;
		private static const DOFILE:int = 3;
		private static const ERROR:int = 4;
		// private static const GCINFO:int = 5;
		private static const GETFENV:int = 6;
		private static const GETMETATABLE:int = 7;
		private static const LOADFILE:int = 8;
		private static const LOAD:int = 9;
		private static const LOADSTRING:int = 10;
		private static const NEXT:int = 11;
		private static const PCALL:int = 12;
		private static const PRINT:int = 13;
		private static const RAWEQUAL:int = 14;
		private static const RAWGET:int = 15;
		private static const RAWSET:int = 16;
		private static const SELECT:int = 17;
		private static const SETFENV:int = 18;
		private static const SETMETATABLE:int = 19;
		private static const TONUMBER:int = 20;
		private static const TOSTRING:int = 21;
		private static const TYPE:int = 22;
		private static const UNPACK:int = 23;
		private static const XPCALL:int = 24;

		private static const IPAIRS:int = 25;
		private static const PAIRS:int = 26;
		private static const IPAIRS_AUX:int = 27;
		private static const PAIRS_AUX:int = 28;

		// The coroutine functions (which reside in the table "coroutine") are also
		// part of the base library.
		private static const CREATE:int = 50;
		private static const RESUME:int = 51;
		private static const RUNNING:int = 52;
		private static const STATUS:int = 53;
		private static const WRAP:int = 54;
		private static const YIELD:int = 55;
		
		private static const WRAP_AUX:int = 56;
		
		/**
		* Lua value that represents the generator function for ipairs.  In
		* PUC-Rio this is implemented as an upvalue of ipairs.
		*/
		private static var IPAIRS_AUX_FUN:Object = new BaseLib(IPAIRS_AUX);
		/**
		* Lua value that represents the generator function for pairs.  In
		* PUC-Rio this is implemented as an upvalue of pairs.
		*/
		private static var PAIRS_AUX_FUN:Object = new BaseLib(PAIRS_AUX);

		/**
		* Which library function this object represents.  This value should
		* be one of the "enums" defined in the class.
		*/
		private var which:int;

		/**
		* For wrapped threads created by coroutine.wrap, this references the
		* Lua thread object.
		*/
		private var thread:Lua;
		
		/** Constructs instance, filling in the 'which' member. */
		public function BaseLib(which:int)
		{
			this.which = which;
		}

		/** Instance constructor used by coroutine.wrap. */
		private function init(L:Lua):void
		{
			this.which = WRAP_AUX;
			this.thread = L;
		}
		
		/**
		 * Implements all of the functions in the Lua base library.  Do not
		 * call directly.
		 * @param L  the Lua state in which to execute.
		 * @return number of returned parameters, as per convention.
		 */
		override public function luaFunction(L:Lua):int
		{
			switch (which)
			{
				case ASSERT:
					return assertFunction(L);
			  
				case COLLECTGARBAGE:
					return collectgarbage(L);
			  
				case DOFILE:
					return dofile(L);
			  
				case ERROR:
					return error(L);
			  
				case GETFENV:
					return getfenv(L);
			  
				case GETMETATABLE:
					return getmetatable(L);
			  
				case IPAIRS:
					return ipairs(L);
			  
				case LOAD:
					return load(L);
			  
				case LOADFILE:
					return loadfile(L);
			  
				case LOADSTRING:
					return loadstring(L);
			  
				case NEXT:
					return next(L);
			  
				case PAIRS:
					return pairs(L);
			  
				case PCALL:
					return pcall(L);
			  
				case PRINT:
					return print(L);
			  
				case RAWEQUAL:
					return rawequal(L);
			  
				case RAWGET:
					return rawget(L);
			  
				case RAWSET:
					return rawset(L);
				
				case SELECT:
					return select(L);
			  
				case SETFENV:
					return setfenv(L);
			  
				case SETMETATABLE:
					return setmetatable(L);
			  
				case TONUMBER:
					return tonumber(L);
			  
				case TOSTRING:
					return tostring(L);
			  
				case TYPE:
					return type(L);
			  
				case UNPACK:
					return unpack(L);
			  
				case XPCALL:
					return xpcall(L);
			  
				case IPAIRS_AUX:
					return ipairsaux(L);
			  
				case PAIRS_AUX:
					return pairsaux(L);

				case CREATE:
					return create(L);
			  
				case RESUME:
					return resume(L);
			  
				case RUNNING:
					return running(L);
			  
				case STATUS:
					return status(L);
			  
				case WRAP:
					return wrap(L);
			  
				case YIELD:
					return yield(L);
			  
				case WRAP_AUX:
					return wrapaux(L);
			}
			return 0;
		}

		/**
		 * Opens the base library into the given Lua state.  This registers
		 * the symbols of the base library in the global table.
		 * @param L  The Lua state into which to open.
		 */
		public static function open(L:Lua):void
		{
			// set global _G
			L.setGlobal("_G", L.getGlobals());
			// set global _VERSION
			L.setGlobal("_VERSION", Lua.VERSION);
			r(L, "assert", ASSERT);
			r(L, "collectgarbage", COLLECTGARBAGE);
			r(L, "dofile", DOFILE);
			r(L, "error", ERROR);
			r(L, "getfenv", GETFENV);
			r(L, "getmetatable", GETMETATABLE);
			r(L, "ipairs", IPAIRS);
			r(L, "loadfile", LOADFILE);
			r(L, "load", LOAD);
			r(L, "loadstring", LOADSTRING);
			r(L, "next", NEXT);
			r(L, "pairs", PAIRS);
			r(L, "pcall", PCALL);
			r(L, "print", PRINT);
			r(L, "rawequal", RAWEQUAL);
			r(L, "rawget", RAWGET);
			r(L, "rawset", RAWSET);
			r(L, "select", SELECT);
			r(L, "setfenv", SETFENV);
			r(L, "setmetatable", SETMETATABLE);
			r(L, "tonumber", TONUMBER);
			r(L, "tostring", TOSTRING);
			r(L, "type", TYPE);
			r(L, "unpack", UNPACK);
			r(L, "xpcall", XPCALL);

			L.__register("coroutine");

			c(L, "create", CREATE);
			c(L, "resume", RESUME);
			c(L, "running", RUNNING);
			c(L, "status", STATUS);
			c(L, "wrap", WRAP);
			c(L, "yield", YIELD);
		}

		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:BaseLib = new BaseLib(which);
			L.setGlobal(name, f);
		}
		
		/** Register a function in the coroutine table. */
		private static function c(L:Lua, name:String, which:int):void
		{
			var f:BaseLib = new BaseLib(which);
			L.setField(L.getGlobal("coroutine"), name, f);
		}
		
		/** Implements assert.  <code>assert</code> is a keyword in some
		 * versions of Java, so this function has a mangled name.
		 */
		private static function assertFunction(L:Lua):int
		{
			L.checkAny(1);
			if (!L.toBoolean(L.value(1)))
			{
				L.error(L.optString(2, "assertion failed!"));
			}
			return L.getTop();
		}

		/** Used by {@link #collectgarbage}. */
		private static var CGOPTS:Array = //new String[]
		[
			"stop", "restart", "collect",
			"count", "step", "setpause", "setstepmul"
		];
		
		/** Used by {@link #collectgarbage}. */
		private static var CGOPTSNUM:Array = //new int[]
		[
			Lua.GCSTOP, Lua.GCRESTART, Lua.GCCOLLECT,
			Lua.GCCOUNT, Lua.GCSTEP, Lua.GCSETPAUSE, Lua.GCSETSTEPMUL
		];
		
		/** Implements collectgarbage. */
		private static function collectgarbage(L:Lua):int
		{
			var o:int = L.checkOption(1, "collect", CGOPTS);
			var ex:int = L.optInt(2, 0);
			var res:int = L.gc(CGOPTSNUM[o], ex);
			switch (CGOPTSNUM[o])
			{
				case Lua.GCCOUNT:
					{
						var b:int = L.gc(Lua.GCCOUNTB, 0);
						L.pushNumber(res + (b as Number)/1024);
						return 1;
					}
			  
				case Lua.GCSTEP:
					L.pushBoolean(res != 0);
					return 1;
				
				default:
					L.pushNumber(res);
					return 1;
			}
		}

		/** Implements dofile. */
		private static function dofile(L:Lua):int
		{
			var fname:String = L.optString(1, null);
			var n:int = L.getTop();
			if (L.loadFile(fname) != 0)
			{
				L.error(L.value(-1));
			}
			L.call(0, Lua.MULTRET);
			return L.getTop() - n;
		}

		/** Implements error. */
		private static function error(L:Lua):int
		{
			var level:int = L.optInt(2, 1);
			L.setTop(1);
			if (Lua.isString(L.value(1)) && level > 0)
			{
				L.insert(L.where(level), 1);
				L.concat(2);
			}
			L.error(L.value(1));
			// NOTREACHED
			return 0;
		}

		/** Helper for getfenv and setfenv. */
		private static function getfunc(L:Lua):Object
		{
			var o:Object = L.value(1);
			if (Lua.isFunction(o))
			{
				return o;
			}
			else
			{
				var level:int = L.optInt(1, 1);
				L.argCheck(level >= 0, 1, "level must be non-negative");
				var ar:Debug = L.getStack(level);
				if (ar == null)
				{
					L.argError(1, "invalid level");
				}
				L.getInfo("f", ar);
				o = L.value(-1);
				if (Lua.isNil(o))
				{
					L.error("no function environment for tail call at level " + level);
				}
				L.pop(1);
				return o;
			}
		}

		/** Implements getfenv. */
		private static function getfenv(L:Lua):int
		{
			var o:Object = getfunc(L);
			if (Lua.isJavaFunction(o))
			{
				L.pushObject(L.getGlobals());
			}
			else
			{
				var f:LuaFunction = o as LuaFunction;
				L.pushObject(f.env);
			}
			return 1;
		}

		/** Implements getmetatable. */
		private static function getmetatable(L:Lua):int
		{
			L.checkAny(1);
			var mt:Object = L.getMetatable(L.value(1));
			if (mt == null)
			{
				L.pushNil();
				return 1;
			}
			var protectedmt:Object = L.getMetafield(L.value(1), "__metatable");
			if (Lua.isNil(protectedmt))
			{
				L.pushObject(mt);               // return metatable
			}
			else
			{
				L.pushObject(protectedmt);      // return __metatable field
			}
			return 1;
		}
			
		/** Implements load. */
		private static function load(L:Lua):int
		{
			var cname:String = L.optString(2, "=(load)");
			L.checkType(1, Lua.TFUNCTION);
			var r:Reader = new BaseLibReader(L, L.value(1));
			var status:int;

			status = L.__load(r, cname);
			return load_aux(L, status);
		}

		/** Implements loadfile. */
		private static function loadfile(L:Lua):int
		{
			var fname:String = L.optString(1, null);
			return load_aux(L, L.loadFile(fname));
		}

		/** Implements loadstring. */
		private static function loadstring(L:Lua):int
		{
			var s:String = L.checkString(1);
			var chunkname:String = L.optString(2, s);
			if (s.substr(0, 1) == "0x1B")//"\033")
			{
				// "binary" dumped into string using string.dump.
				return load_aux(L, L.load(new DumpedInput(s), chunkname));
			}
			else
			{
				return load_aux(L, L.loadString(s, chunkname));
			}
		}
			
		private static function load_aux(L:Lua, status:int):int
		{
			if (status == 0)    // OK?
			{
				return 1;
			}
			else
			{
				L.insert(Lua.NIL, -1);      // put before error message
				return 2; // return nil plus error message
			}
		}

		/** Implements next. */
		private static function next(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			L.setTop(2);        // Create a 2nd argument is there isn't one
			if (L.next(1))
			{
				return 2;
			}
			L.pushObject(Lua.NIL);
			return 1;
		}

		/** Implements ipairs. */
		private static function ipairs(L:Lua):int 
		{
			L.checkType(1, Lua.TTABLE);
			L.pushObject(IPAIRS_AUX_FUN);
			L.pushValue(1);
			L.pushNumber(0);
			return 3;
		}

		/** Generator for ipairs. */
		private static function ipairsaux(L:Lua):int
		{
			var i:int = L.checkInt(2);
			L.checkType(1, Lua.TTABLE);
			++i;
			var v:Object = Lua.rawGetI(L.value(1), i);
			if (Lua.isNil(v))
			{
				return 0;
			}
			L.pushNumber(i);
			L.pushObject(v);
			return 2;
		}

		/** Implements pairs.  PUC-Rio uses "next" as the generator for pairs.
		 * Jill doesn't do that because it would be way too slow.  We use the
		 * {@link java.util.Enumeration} returned from
		 * {@link java.util.Hashtable#keys}.  The {@link #pairsaux} method
		 * implements the step-by-step iteration.
		 */
		private static function pairs(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			L.pushObject(PAIRS_AUX_FUN);                   // return generator,
			var t:LuaTable = L.value(1) as LuaTable;
			L.pushObject([t, t.keys()]); //TODO:   				 // state,
			L.pushObject(Lua.NIL);                         // and initial value.
			return 3;
		}

		/** Generator for pairs.  This expects a <var>state</var> and
		 * <var>var</var> as (Lua) arguments.
		 * The state is setup by {@link #pairs} and is a
		 * pair of {LuaTable, Enumeration} stored in a 2-element array.  The
		 * <var>var</var> is not used.  This is in contrast to the PUC-Rio
		 * implementation, where the state is the table, and the var is used
		 * to generated the next key in sequence.  The implementation, of
		 * pairs and pairsaux, has no control over <var>var</var>,  Lua's
		 * semantics of <code>for</code> force it to be the previous result
		 * returned by this function.  In Jill this value is not suitable to
		 * use for enumeration, which is why it isn't used.
		 */
		private static function pairsaux(L:Lua):int
		{
			var a:Array = L.value(1) as Array; //(Object[])
			var t:LuaTable = a[0] as LuaTable;
			var e:Enumeration = a[1] as Enumeration;
			if (!e.hasMoreElements())
			{
				return 0;
			}
			var key:Object = e.nextElement();
			L.pushObject(key);
			L.pushObject(t.getlua(key));
			return 2;
		}

		/** Implements pcall. */
		private static function pcall(L:Lua):int
		{
			L.checkAny(1);
			var status:int = L.pcall(L.getTop()-1, Lua.MULTRET, null);
			var b:Boolean = (status == 0);
			L.insert(Lua.valueOfBoolean(b), 1);
			return L.getTop();
		}
			
		/**
		 * The {@link PrintStream} used by print.  Makes it more convenient if
		 * redirection is desired.  For example, client code could implement
		 * their own instance which sent output to the screen of a JME device.
		 */
		public static var OUT:PrintStream = SystemUtil.out;

		/** Implements print. */
		private static function print(L:Lua):int
		{
			var n:int = L.getTop();
			var tostring:Object = L.getGlobal("tostring");
			for (var i:int = 1; i <= n; ++i)
			{
				L.pushObject(tostring);
				L.pushValue(i);
				L.call(1, 1);
				var s:String = L.toString(L.value(-1));
				if (s == null)
				{
					return L.error("'tostring' must return a string to 'print'");
				}
				if (i>1)
				{
					OUT.print('\t');
				}
				OUT.print(s);
				L.pop(1);
			}
			OUT.println();
			return 0;
		}
			
		/** Implements rawequal. */
		private static function rawequal(L:Lua):int
		{
			L.checkAny(1);
			L.checkAny(2);
			L.pushBoolean(Lua.rawEqual(L.value(1), L.value(2)));
			return 1;
		}

		/** Implements rawget. */
		private static function rawget(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			L.checkAny(2);
			L.pushObject(Lua.rawGet(L.value(1), L.value(2)));
			return 1;
		}

		/** Implements rawset. */
		private static function rawset(L:Lua):int 
		{
			L.checkType(1, Lua.TTABLE);
			L.checkAny(2);
			L.checkAny(3);
			L.rawSet(L.value(1), L.value(2), L.value(3));
			return 0;
		}

		/** Implements select. */
		private static function select(L:Lua):int
		{
			var n:int = L.getTop();
			if (L.type(1) == Lua.TSTRING && "#" == L.toString(L.value(1)))
			{
				L.pushNumber(n-1);
				return 1;
			}
			var i:int = L.checkInt(1);
			if (i < 0)
			{
				i = n + i;
			}
			else if (i > n)
			{
			  i = n;
			}
			L.argCheck(1 <= i, 1, "index out of range");
			return n-i;
		}
		
		/** Implements setfenv. */
		private static function setfenv(L:Lua):int
		{
			L.checkType(2, Lua.TTABLE);
			var o:Object = getfunc(L);
			var first:Object = L.value(1);
			if (Lua.isNumber(first) && L.toNumber(first) == 0)
			{
				// :todo: change environment of current thread.
				return 0;
			}
			else if (Lua.isJavaFunction(o) || !L.setFenv(o, L.value(2)))
			{
				L.error("'setfenv' cannot change environment of given object");
			}
			L.pushObject(o);
			return 1;
		}
		
		/** Implements setmetatable. */
		private static function setmetatable(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			var t:int = L.type(2);
			L.argCheck(t == Lua.TNIL || t == Lua.TTABLE, 2,
				"nil or table expected");
			if (!Lua.isNil(L.getMetafield(L.value(1), "__metatable")))
			{
				L.error("cannot change a protected metatable");
			}
			L.setMetatable(L.value(1), L.value(2));
			L.setTop(1);
			return 1;
		}

		/** Implements tonumber. */
		private static function tonumber(L:Lua):int
		{
			var base:int = L.optInt(2, 10);
			if (base == 10)     // standard conversion
			{
				L.checkAny(1);
				var o:Object = L.value(1);
				if (Lua.isNumber(o))
				{
					L.pushNumber(L.toNumber(o));
					return 1;
				}
			}
			else
			{
				var s:String = L.checkString(1);
				L.argCheck(2 <= base && base <= 36, 2, "base out of range");
				// :todo: consider stripping space and sharing some code with
				// Lua.vmTostring
				try
				{
					var i:int = int(s);//Integer.parseInt(s, base); //TODO:
					L.pushNumber(i);
					return 1;
				}
				catch (e_:NumberFormatException)
				{
					trace(e_.getStackTrace());
				}
			}
			L.pushObject(Lua.NIL);
			return 1;
		}
		
		/** Implements tostring. */
		private static function tostring(L:Lua):int
		{
			L.checkAny(1);
			var o:Object = L.value(1);

			if (L.callMeta(1, "__tostring"))    // is there a metafield?
			{
				return 1; // use its value
			}
			switch (L.type(1))
			{
				case Lua.TNUMBER:
					L.pushString(L.toString(o));
					break;
			  
				case Lua.TSTRING:
					L.pushObject(o);
					break;
			  
				case Lua.TBOOLEAN:
					if (L.toBoolean(o))
					{
						L.pushLiteral("true");
					}
					else
					{
						L.pushLiteral("false");
					}
					break;
				
				case Lua.TNIL:
					L.pushLiteral("nil");
					break;
			  
				default:
					L.pushString(o.toString());
					break;
			}
			return 1;
		}
		
		/** Implements type. */
		private static function type(L:Lua):int
		{
			L.checkAny(1);
			L.pushString(L.typeNameOfIndex(1));
			return 1;
		}
			
		/** Implements unpack. */
		private static function unpack(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			var t:LuaTable = L.value(1) as LuaTable;
			var i:int = L.optInt(2, 1);
			var e:int = L.optInt(3, t.getn());
			var n:int = e - i + 1;  // number of elements
			if (n <= 0)
			{
				return 0;         // empty range
			}
			// i already initialised to start index, which isn't necessarily 1
			for (; i <= e; ++i)
			{
				L.pushObject(t.getnum(i));
			}
			return n;
		}
		
		/** Implements xpcall. */
		private static function xpcall(L:Lua):int
		{
			L.checkAny(2);
			var errfunc:Object = L.value(2);
			L.setTop(1);        // remove error function from stack
			var status:int = L.pcall(0, Lua.MULTRET, errfunc);
			L.insert(Lua.valueOfBoolean(status == 0), 1);
			return L.getTop();  // return status + all results
		}
			
		/** Implements coroutine.create. */
		private static function create(L:Lua):int
		{
			var NL:Lua = L.newThread();
			var faso:Object = L.value(1);
			L.argCheck(Lua.isFunction(faso) && !Lua.isJavaFunction(faso), 1,
				"Lua function expected");
			L.setTop(1);        // function is at top
			L.xmove(NL, 1);     // move function from L to NL
			L.pushObject(NL);
			return 1;
		}
		
		/** Implements coroutine.resume. */
		private static function resume(L:Lua):int
		{
			var co:Lua = L.toThread(L.value(1));
			L.argCheck(co != null, 1, "coroutine expected");
			var r:int = auxresume(L, co, L.getTop() - 1);
			if (r < 0)
			{
				L.insert(Lua.valueOfBoolean(false), -1);
				return 2; // return false + error message
			}
			L.insert(Lua.valueOfBoolean(true), L.getTop()-(r-1));
			return r + 1;       // return true + 'resume' returns
		}

		/** Implements coroutine.running. */
		private static function running(L:Lua):int
		{
			if (L.isMain())
			{
				return 0; // main thread is not a coroutine
			}
			L.pushObject(L);
			return 1;
		}
		
		/** Implements coroutine.status. */
		private static function status(L:Lua):int
		{
			var co:Lua = L.toThread(L.value(1));
			L.argCheck(co != null, 1, "coroutine expected");
			if (L == co)
			{
				L.pushLiteral("running");
			}
			else
			{
				switch (co.status)
				{
					case Lua.YIELD:
						L.pushLiteral("suspended");
						break;
				
					case 0:
						{
							var ar:Debug = co.getStack(0);
							if (ar != null)       // does it have frames?
							{
								L.pushLiteral("normal");    // it is running
							}
							else if (co.getTop() == 0)
							{
								L.pushLiteral("dead");
							}
							else
							{
								L.pushLiteral("suspended"); // initial state
							}
						}
						break;
				
					default:        // some error occured
						L.pushLiteral("dead");
				}
			}
			return 1;
		}

		/** Implements coroutine.wrap. */
		private static function wrap(L:Lua):int
		{
			create(L);
			L.pushObject(wrapit(L.toThread(L.value(-1))));
			return 1;
		}

		/** Helper for wrap.  Returns a LuaJavaCallback that has access to the
		 * Lua thread.
		 * @param L the Lua thread to be wrapped.
		 */
		private static function wrapit(L:Lua):LuaJavaCallback
		{
			var lib:BaseLib = new BaseLib(0);
			lib.init(L);
			return lib;
		}

		/** Helper for wrap.  This implements the function returned by wrap. */
		private function wrapaux(L:Lua):int
		{
			var co:Lua = thread;
			var r:int = auxresume(L, co, L.getTop());
			if (r < 0)
			{
				if (Lua.isString(L.value(-1)))      // error object is a string?
				{
					var w:String = L.where(1);
					L.insert(w, -1);
					L.concat(2);
				}
				L.error(L.value(-1));     // propagate error
			}
			return r;
		}
		
		private static function auxresume(L:Lua, co:Lua, narg:int):int
		{
			// if (!co.checkStack...
			if (co.status == 0 && co.getTop() == 0)
			{
				L.pushLiteral("cannot resume dead coroutine");
				return -1;        // error flag;
			}
			L.xmove(co, narg);
			var status:int = co.resume(narg);
			if (status == 0 || status == Lua.YIELD)
			{
				var nres:int = co.getTop();
				// if (!L.checkStack...
				co.xmove(L, nres);        // move yielded values
				return nres;
			}
			co.xmove(L, 1);   // move error message
			return -1;        // error flag;
		}
		
		/** Implements coroutine.yield. */
		private static function yield(L:Lua):int
		{
			return L.yield(L.getTop());
		}
	}
}