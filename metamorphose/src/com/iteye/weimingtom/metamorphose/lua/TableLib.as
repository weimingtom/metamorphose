/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/TableLib.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	
	/**
	 * Contains Lua's table library.
	 * The library can be opened using the {@link #open} method.
	 */
	public final class TableLib extends LuaJavaCallback
	{
		
		// Each function in the table library corresponds to an instance of
		// this class which is associated (the 'which' member) with an integer
		// which is unique within this class.  They are taken from the following
		// set.  
		
		private static const CONCAT:int = 1;
		private static const INSERT:int = 2;
		private static const MAXN:int = 3;
		private static const REMOVE:int = 4;
		private static const SORT:int = 5;
		private static const GETN:int = 6;
  
		/**
		 * Which library function this object represents.  This value should
		 * be one of the "enums" defined in the class.
		 */
		private var _which:int;

		/** Constructs instance, filling in the 'which' member. */
		public function TableLib(which:int) 
		{
			this._which = which;
		}
		
		/**
		 * Implements all of the functions in the Lua table library.  Do not
		 * call directly.
		 * @param L  the Lua state in which to execute.
		 * @return number of returned parameters, as per convention.
		 */
		override public function luaFunction(L:Lua):int
		{
			switch (this._which)
			{
				case CONCAT:
					return concat(L);
		  
				case INSERT:
					return insert(L);
		  
				case MAXN:
					return maxn(L);
		  
				case REMOVE:
					return remove(L);
		  
				case SORT:
					return sort(L);
					
				//FIXME: added
				case GETN:
					return getn(L);	
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
			L.__register("table");

			r(L, "concat", CONCAT);
			r(L, "insert", INSERT);
			r(L, "getn", GETN); //FIXME: added
			r(L, "maxn", MAXN);
			r(L, "remove", REMOVE);
			r(L, "sort", SORT);
		}

		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:TableLib = new TableLib(which);
			var lib:Object = L.getGlobal("table");
			L.setField(lib, name, f);
		}

		/** Implements table.concat. */
		private static function concat(L:Lua):int
		{
			var sep:String = L.optString(2, "");
			L.checkType(1, Lua.TTABLE);
			var i:int = L.optInt(3, 1);
			var last:int = L.optInt(4, Lua.objLen(L.value(1)));
			var b:StringBuffer = new StringBuffer();
			var t:Object = L.value(1);
			for (; i <= last; ++i)
			{
				var v:Object = Lua.rawGetI(t, i);
				L.argCheck(Lua.isString(v), 1, "table contains non-strings");
				b.appendString(L.toString(v));
				if (i != last)
					b.appendString(L.toString(sep));
			}
			L.pushString(b.toString());
			return 1;
		}

		/** Implements table.insert. */
		private static function insert(L:Lua):int
		{
			var e:int = aux_getn(L, 1) + 1; // first empty element
			var pos:int;    // where to insert new element
			var t:Object = L.value(1);
		
			switch (L.getTop())
			{
				case 2:   // called with only 2 arguments
					pos = e;        // insert new element at the end
					break;

				case 3:
					{
						var i:int;
						pos = L.checkInt(2);  // 2nd argument is the position
						if (pos > e)
							e = pos;    // grow array if necessary
						for (i = e; i > pos; --i)     // move up elements
						{
							// t[i] = t[i-1]
							L.rawSetI(t, i, Lua.rawGetI(t, i-1));
						}
					}
					break;

				default:
					return L.error("wrong number of arguments to 'insert'");
			}
			L.rawSetI(t, pos, L.value(-1));     // t[pos] = v
			return 0;
		}

		/** Implements table.maxn. */
		private static function maxn(L:Lua):int
		{
			var max:Number = 0;
			L.checkType(1, Lua.TTABLE);
			var t:LuaTable = L.value(1) as LuaTable;
			var e:Enumeration = t.keys();
			while (e.hasMoreElements())
			{
				var o:Object = e.nextElement();
				if (Lua.____type(o) == Lua.TNUMBER)
				{
					var v:Number = L.toNumber(o);
					if (v > max)
					max = v;
				}
			}
			L.pushNumber(max);
			return 1;
		}

		/** Implements table.remove. */
		private static function remove(L:Lua):int
		{
			var e:int = aux_getn(L, 1);
			var pos:int = L.optInt(2, e);
			if (e == 0)
				return 0;         // table is 'empty'
			var t:Object = L.value(1);
			var o:Object = Lua.rawGetI(t, pos);       // result = t[pos]
			for ( ; pos < e; ++pos)
			{
				L.rawSetI(t, pos, Lua.rawGetI(t, pos+1));   // t[pos] = t[pos+1]
			}
			L.rawSetI(t, e, Lua.NIL);   // t[e] = nil
			L.pushObject(o);
			return 1;
		}

		/** Implements table.sort. */
		private static function sort(L:Lua):int
		{
			var n:int = aux_getn(L, 1);
			if (!L.isNoneOrNil(2))      // is there a 2nd argument?
				L.checkType(2, Lua.TFUNCTION);
			L.setTop(2);        // make sure there is two arguments
			auxsort(L, 1, n);
			return 0;
		}

		public static function auxsort(L:Lua, l:int, u:int):void
		{
			var t:Object = L.value(1);
			while (l < u)       // for tail recursion
			{
				var i:int;
				var j:int;
				// sort elements a[l], a[l+u/2], and a[u]
				var o1:Object = Lua.rawGetI(t, l);
				var o2:Object = Lua.rawGetI(t, u);
				if (sort_comp(L, o2, o1)) // a[u] < a[l]?
				{	
					L.rawSetI(t, l, o2);
				    L.rawSetI(t, u, o1);
				}
				if (u-l == 1)
					break;  // only 2 elements
				i = (l+u)/2;
				o1 = Lua.rawGetI(t, i);
				o2 = Lua.rawGetI(t, l);
				if (sort_comp(L, o1, o2)) // a[i]<a[l]?
				{
					L.rawSetI(t, i, o2);
					L.rawSetI(t, l, o1);
				}
				else
				{
					o2 = Lua.rawGetI(t, u);
					if (sort_comp(L, o2, o1))       // a[u]<a[i]?
					{
						L.rawSetI(t, i, o2);
						L.rawSetI(t, u, o1);
					}
				}
				if (u-l == 2)
					break;  // only 3 elements
				var p:Object = Lua.rawGetI(t, i); // Pivot
				o2 = Lua.rawGetI(t, u-1);
				L.rawSetI(t, i, o2);
				L.rawSetI(t, u-1, p);
				// a[l] <= P == a[u-1] <= a[u], only need to sort from l+1 to u-2
				i = l;
				j = u-1;
				// NB: Pivot P is in p
				while (true)      // invariant: a[l..i] <= P <= a[j..u]
				{
					// repeat ++i until a[i] >= P
					while (true)
					{
						o1 = Lua.rawGetI(t, ++i);
						if (!sort_comp(L, o1, p))
							break;
						if (i>u)
							L.error("invalid order function for sorting");
					}
					// repreat --j until a[j] <= P
					while (true)
					{
						o2 = Lua.rawGetI(t, --j);
						if (!sort_comp(L, p, o2))
							break;
						if (j<l)
							L.error("invalid order function for sorting");
					}
					if (j < i)
						break;
					L.rawSetI(t, i, o2);
					L.rawSetI(t, j, o1);
				}
				o1 = Lua.rawGetI(t, u-1);
				o2 = Lua.rawGetI(t, i);
				L.rawSetI(t, u-1, o2);
				L.rawSetI(t, i, o1);      // swap pivot (a[u-1]) with a[i]
				// a[l..i-1 <= a[i] == P <= a[i+1..u]
				// adjust so that smaller half is in [j..i] and larger one in [l..u]
				if (i-l < u-i)
				{
					j = l;
					i = i - 1;
					l = i + 2;
				}
				else
				{
					j = i + 1;
					i = u;
					u = j - 2;
				}
				auxsort(L, j, i); // call recursively the smaller one
			} // repeat the routine for the larger one
		}

		private static function sort_comp(L:Lua, a:Object, b:Object):Boolean
		{
			if (!Lua.isNil(L.value(2)))   // function?
			{
				L.pushValue(2);
				L.pushObject(a);
				L.pushObject(b);
				L.call(2, 1);
				var res:Boolean = L.toBoolean(L.value(-1));
				L.pop(1);
				return res;
			}
			else        // a < b?
			{
				return L.lessThan(a, b);
			}
		}

		private static function aux_getn(L:Lua, n:int):int
		{
			L.checkType(n, Lua.TTABLE);
			var t:LuaTable = L.value(n) as LuaTable;
			return t.getn();
		}
		
		//FIXME: added
		private static function getn(L:Lua):int 
		{
			L.pushNumber(aux_getn(L, 1));
			return 1;
		}
	}
}