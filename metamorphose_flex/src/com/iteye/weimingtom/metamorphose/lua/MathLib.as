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

//see jillcode(Java Implementation of Lua Language, Jill):
//	http://code.google.com/p/jillcode/
//这里的代码移植自jillcode(Lua的Java实现，Jill):
//	http://code.google.com/p/jillcode/	
package com.iteye.weimingtom.metamorphose.lua 
{
	import com.iteye.weimingtom.metamorphose.java.Random;
	import com.iteye.weimingtom.metamorphose.java.MathUtil;

	/**
	 * Contains Lua's math library.
	 * The library can be opened using the {@link #open} method.
	 * Because this library is implemented on top of CLDC 1.1 it is not as
	 * complete as the PUC-Rio math library.  Trigononmetric inverses
	 * (EG <code>acos</code>) and hyperbolic trigonometric functions (EG
	 * <code>cosh</code>) are not provided.
	 */
	public final class MathLib
	{
		// Each function in the library corresponds to an instance of
		// this class which is associated (the 'which' member) with an integer
		// which is unique within this class.  They are taken from the following
		// set.
		
		private static const ABS:int = 1;
		//private static const acos:int = 2;
		//private static const asin:int = 3;
		//private static const atan2:int = 4;
		//private static const atan:int = 5;
		private static const CEIL:int = 6;
		//private static const cosh:int = 7;
		private static const COS:int = 8;
		private static const DEG:int = 9;
		private static const EXP:int = 10;
		private static const FLOOR:int = 11;
		private static const FMOD:int = 12;
		//private static const frexp:int = 13;
		//private static const ldexp:int = 14;
		//private static const log:int = 15;
		private static const MAX:int = 16;
		private static const MIN:int = 17;
		private static const MODF:int = 18;
		private static const POW:int = 19;
		private static const RAD:int = 20;
		private static const RANDOM:int = 21;
		private static const RANDOMSEED:int = 22;
		//private static const sinh:int = 23;
		private static const SIN:int = 24;
		private static const SQRT:int = 25;
		//private static const tanh:int = 26;
		private static const TAN:int = 27;

		private static var _rng:Random = new Random();

		/**
		* Which library function this object represents.  This value should
		* be one of the "enums" defined in the class.
		*/
		private var _which:int;

		/** Constructs instance, filling in the 'which' member. */
		public function MathLib(which:int) 
		{
			this._which = which;
		}
		
		/**
		 * Implements all of the functions in the Lua math library.  Do not
         * call directly.
         * @param L  the Lua state in which to execute.
         * @return number of returned parameters, as per convention.
         */
		public function luaFunction(L:Lua):int
		{
			switch (this._which)
			{
				case ABS:
					return abs(L);
      
				case CEIL:
					return ceil(L);
      
				case COS:
					return cos(L);
      
				case DEG:
					return deg(L);
      
				case EXP:
					return exp(L);
      
				case FLOOR:
					return floor(L);
      
				case FMOD:
					return fmod(L);
      
				case MAX:
					return max(L);
      
				case MIN:
					return min(L);
      
				case MODF:
					return modf(L);
      
				case POW:
					return pow(L);
      
				case RAD:
					return rad(L);
      
				case RANDOM:
					return random(L);
      
				case RANDOMSEED:
					return randomseed(L);
      
				case SIN:
					return sin(L);
      
				case SQRT:
					return sqrt(L);
				
				case TAN:
					return tan(L);
			}
			return 0;
		}

		/**
		 * Opens the library into the given Lua state.  This registers
         * the symbols of the library in the global table.
         * @param L  The Lua state into which to open.
         */
		public static function open(L:Lua):void
		{
			var t:LuaTable = L.__register("math");

			r(L, "abs", ABS);
			r(L, "ceil", CEIL);
			r(L, "cos", COS);
			r(L, "deg", DEG);
			r(L, "exp", EXP);
			r(L, "floor", FLOOR);
			r(L, "fmod", FMOD);
			r(L, "max", MAX);
			r(L, "min", MIN);
			r(L, "modf", MODF);
			r(L, "pow", POW);
			r(L, "rad", RAD);
			r(L, "random", RANDOM);
			r(L, "randomseed", RANDOMSEED);
			r(L, "sin", SIN);
			r(L, "sqrt", SQRT);
			r(L, "tan", TAN);
			
			L.setField(t, "pi", Lua.valueOfNumber(Math.PI));
			L.setField(t, "huge", Lua.valueOfNumber(Number.POSITIVE_INFINITY));
		}
		
		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:MathLib = new MathLib(which);
			L.setField(L.getGlobal("math"), name, f);
		}

		private static function abs(L:Lua):int
		{
			L.pushNumber(Math.abs(L.checkNumber(1)));
			return 1;
		}

		private static function ceil(L:Lua):int
		{
			L.pushNumber(Math.ceil(L.checkNumber(1)));
			return 1;
		}

		private static function cos(L:Lua):int
		{
			L.pushNumber(Math.cos(L.checkNumber(1)));
			return 1;
		}

		private static function deg(L:Lua):int
		{
			L.pushNumber(MathUtil.toDegrees(L.checkNumber(1)));
			return 1;
		}

		private static function exp(L:Lua):int 
		{
			// CLDC 1.1 has Math.E but no exp, pow, or log.  Bizarre.
			L.pushNumber(Lua.iNumpow(Math.E, L.checkNumber(1)));
			return 1;
		}

		private static function floor(L:Lua):int
		{
			L.pushNumber(Math.floor(L.checkNumber(1)));
			return 1;
		}

		private static function fmod(L:Lua):int 
		{
			L.pushNumber(L.checkNumber(1) % L.checkNumber(2));
			return 1;
		}

		private static function max(L:Lua):int
		{
			var n:int = L.getTop(); // number of arguments
			var dmax:Number = L.checkNumber(1);
			for (var i:int = 2; i <= n; ++i)
			{
				var d:Number = L.checkNumber(i);
				dmax = Math.max(dmax, d);
			}
			L.pushNumber(dmax);
			return 1;
		}

		private static function min(L:Lua):int
		{
			var n:int = L.getTop(); // number of arguments
			var dmin:Number = L.checkNumber(1);
			for (var i:int=2; i<=n; ++i)
			{
				var d:Number = L.checkNumber(i);
				dmin = Math.min(dmin, d);
			}
			L.pushNumber(dmin);
			return 1;
		}

		private static function modf(L:Lua):int
		{
			var x:Number = L.checkNumber(1);
			var fp:Number = x % 1;
			var ip:Number = x - fp;
			L.pushNumber(ip);
			L.pushNumber(fp);
			return 2;
		}

		private static function pow(L:Lua):int
		{
			L.pushNumber(Lua.iNumpow(L.checkNumber(1), L.checkNumber(2)));
			return 1;
		}
		
		private static function rad(L:Lua):int
		{
			L.pushNumber(MathUtil.toRadians(L.checkNumber(1)));
			return 1;
		}

		private static function random(L:Lua):int
		{
			// It would seem better style to associate the java.util.Random
			// instance with the Lua instance (by implementing and using a
			// registry for example).  However, PUC-rio uses the ISO C library
			// and so will share the same random number generator across all Lua
			// states.  So we do too.
			switch (L.getTop()) // check number of arguments
			{
				case 0:   // no arguments
					L.pushNumber(MathLib._rng.nextDouble());
					break;
		  
				case 1:   // only upper limit
					{
						var u:int = L.checkInt(1);
						L.argCheck(1 <= u, 1, "interval is empty");
						L.pushNumber(MathLib._rng.nextInt(u) + 1);
					}
					break;

				case 2:   // lower and upper limits
					{
						var l:int = L.checkInt(1);
						var u2:int = L.checkInt(2);
						L.argCheck(l <= u2, 2, "interval is empty");
						L.pushNumber(MathLib._rng.nextInt(u2) + l);
					}
					break;

				default:
					return L.error("wrong number of arguments");
			}
			return 1;
		}

		private static function randomseed(L:Lua):int
		{
			MathLib._rng.setSeed(L.checkNumber(1) as Number);
			return 0;
		}

		private static function sin(L:Lua):int
		{
			L.pushNumber(Math.sin(L.checkNumber(1)));
			return 1;
		}
		
		private static function sqrt(L:Lua):int
		{
			L.pushNumber(Math.sqrt(L.checkNumber(1)));
			return 1;
		}

		private static function tan(L:Lua):int
		{
			L.pushNumber(Math.tan(L.checkNumber(1)));
			return 1;
		}	
	}
}