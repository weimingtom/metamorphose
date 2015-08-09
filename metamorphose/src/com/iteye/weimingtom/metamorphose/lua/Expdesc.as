/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/Expdesc.java#1 $
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
	/** Equivalent to struct expdesc. */
	public final class Expdesc extends Object
	{
		public static const VVOID:int = 0;           // no value
		public static const VNIL:int = 1;
		public static const VTRUE:int = 2;
		public static const VFALSE:int = 3;
		public static const VK:int = 4;              // info = index into 'k'
		public static const VKNUM:int = 5;           // nval = numerical value
		public static const VLOCAL:int = 6;          // info = local register
		public static const VUPVAL:int = 7;          // info = index into 'upvalues'
		public static const VGLOBAL:int = 8;         // info = index of table;
												     // aux = index of global name in 'k'
		public static const VINDEXED:int = 9;        // info = table register
												     // aux = index register (or 'k')
		public static const VJMP:int = 10;           // info = instruction pc
		public static const VRELOCABLE:int = 11;     // info = instruction pc
		public static const VNONRELOC:int = 12;      // info = result register
		public static const VCALL:int = 13;          // info = instruction pc
		public static const VVARARG:int = 14;        // info = instruction pc

		private var _k:int;        // one of V* enums above
		private var _info:int;
		private var _aux:int;
		private var _nval:Number;
		private var _t:int;
		private var _f:int;
		
		//TODO:
		public function Expdesc() 
		{
			
		}
		
		//public function Expdesc(k:int, i:int):void
		//{
			//init(k, i);
		//}

		/** Equivalent to init_exp from lparser.c */
		public function init(kind:int, i:int):void
		{
			this._t = FuncState.NO_JUMP;
			this._f = FuncState.NO_JUMP;
			this._k = kind;
			this._info = i;
		}
		
		public function copy(e:Expdesc):void
		{
			// Must initialise all members of this.
			this._k = e._k;
			this._info = e._info;
			this._aux = e._aux;
			this._nval = e._nval;
			this._t = e._t;
			this._f = e._f;
		}

		public function get kind():int
		{
			return this._k;
		}

		public function set kind(kind:int):void
		{
			this._k = kind;
		}

		public function get k():int
		{
			return this._k;
		}

		public function set k(kind:int):void
		{
			this._k = kind;
		}
		
		public function get info():int
		{
			return this._info;
		}

		public function set info(i:int):void
		{
			this._info = i;
		}

		public function get aux():int
		{
			return this._aux;
		}
		
		public function set aux(aux:int):void
		{
			this._aux = aux;
		}
		
		public function get nval():Number
		{
			return this._nval;
		}

		public function set nval(d:Number):void
		{
			this._nval = d;
		}

		/** Equivalent to hasmultret from lparser.c */
		public function hasmultret():Boolean
		{
			return this._k == VCALL || this._k == VVARARG;
		}

		/** Equivalent to hasjumps from lcode.c. */
		public function hasjumps():Boolean
		{
			return this._t != this._f;
		}

		public function nonreloc(i:int):void
		{
			this._k = VNONRELOC;
			this._info = i;
		}

		public function reloc(i:int):void
		{
			this._k = VRELOCABLE;
			this._info = i;
		}

		public function upval(i:int):void
		{
			this._k = VUPVAL;
			this._info = i;
		}
		
		//新增
		public function get f():int
		{
			return this._f;
		}
		
		//新增
		public function set f(f:int):void
		{
			this._f = f;
		}

		//新增
		public function get t():int
		{
			return this._t;
		}
		
		//新增
		public function set t(t:int):void
		{
			this._t = t;
		}
		
	}
}