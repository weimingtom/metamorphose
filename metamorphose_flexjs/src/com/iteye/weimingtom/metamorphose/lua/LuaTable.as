/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/LuaTable.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.Hashtable;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;
	
	/**
	 * Class that models Lua's tables.  Each Lua table is an instance of
	 * this class.  Whilst you can clearly see that this class extends
	 * {@link java.util.Hashtable} you should in no way rely upon that.
	 * Calling any methods that are not defined in this class (but are
	 * defined in a super class) is extremely deprecated.
	 */
	public final class LuaTable extends Hashtable
	{
		private static const MAXBITS:int = 26;
		private static const MAXASIZE:int = 1 << MAXBITS;

		private var _metatable:LuaTable;   // = null;
		private static const ZERO:Array = new Array(0); //final Object[]

		/**
		 * Array used so that tables accessed like arrays are more efficient.
		 * All elements stored at an integer index, <var>i</var>, in the
		 * range [1,sizeArray] are stored at <code>array[i-1]</code>.
		 * This speed and space usage for array-like access.
		 * When the table is rehashed the array's size is chosen to be the
		 * largest power of 2 such that at least half the entries are
		 * occupied.  Default access granted for {@link Enum} class, do not
		 * abuse.
		 */
		private var _array:Array = ZERO; //Object[]
		/**
		 * Equal to <code>array.length</code>.  Default access granted for
		 * {@link Enum} class, do not abuse.
		 */
		private var _sizeArray:int;        // = 0;
		/**
		 * <code>true</code> whenever we are in the {@link #rehash}
		 * method.  Avoids infinite rehash loops.
		 */
		private var _inrehash:Boolean;     // = false;
  
		public function LuaTable() 
		{
			//TODO:暂时屏蔽
			super(1);
		}
		
		/**
		 * Fresh LuaTable with hints for preallocating to size.
		 * @param narray  number of array slots to preallocate.
		 * @param nhash   number of hash slots to preallocate.
		 */
		public function init(narray:int, nhash:int):void
		{
			// :todo: super(nhash) isn't clearly correct as adding nhash hash
			// table entries will causes a rehash with the usual implementation
			// (which rehashes when ratio of entries to capacity exceeds the
			// load factor of 0.75).  Perhaps ideally we would size the hash
			// tables such that adding nhash entries will not cause a rehash.
			
			//TODO:
			//super(nhash); 
			
			this._array = new Array(narray);
			for (var i:int = 0; i < narray; ++i)
			{
				this._array[i] = Lua.NIL;
			}
			this._sizeArray = narray;
		}

		/**
		 * Implements discriminating equality.  <code>o1.equals(o2) == (o1 ==
		 * o2) </code>.  This method is not necessary in CLDC, it's only
		 * necessary in J2SE because java.util.Hashtable overrides equals.
		 * @param o  the reference to compare with.
		 * @return true when equal.
		 */
		public function equals(o:Object):Boolean
		{
			return this == o;
		}

		/**
		 * Provided to avoid Checkstyle warning.  This method is not necessary
		 * for correctness (in neither JME nor JSE), it's only provided to
		 * remove a Checkstyle warning.
		 * Since {@link #equals} implements the most discriminating
		 * equality possible, this method can have any implementation.
		 * @return an int.
		*/
		public function hashCode():int
		{
			return SystemUtil.identityHashCode(this);
		}

		private static function arrayindex(key:Object):int
		{
			if (key is Number)
			{
				var d:Number = key as Number;
				var k:int = d as int;
				if (k == d)
				{
					return k;
				}
			}
			return -1;  // 'key' did not match some condition
		}

		private static function computesizes(nums:Array/*int[] */, narray:Array/*int[] */):int
		{
			var t:int = narray[0] as int;
			var a:int = 0;  // number of elements smaller than 2^i
			var na:int = 0; // number of elements to go to array part
			var n:int = 0;  // optimal size for array part
			var twotoi:int = 1;     // 2^i
			for (var i:int = 0; twotoi / 2 < t; ++i)
			{
				if (nums[i] > 0)
				{
					a += nums[i];
					if (a > twotoi/2)       // more than half elements present?
					{
						n = twotoi;   // optimal size (till now)
						na = a;       // all elements smaller than n will go to array part
					}
				}
				if (a == t)       // all elements already counted
				{
					break;
				}
				twotoi *= 2;
			}
			narray[0] = n;
			//# assert narray[0]/2 <= na && na <= narray[0]
			return na;
		}

		private function countint(key:Object, nums:Array/*int[] */):int
		{
			var k:int = arrayindex(key);
			if (0 < k && k <= MAXASIZE) // is 'key' an appropriate array index?
			{
			  ++nums[ceillog2(k)];      // count as such
			  return 1;
			}
			return 0;
		}

		private function numusearray(nums:Array /*int[] */):int
		{
			var ause:int = 0;       // summation of 'nums'
			var i:int = 1;  // count to traverse all array keys
			var ttlg:int = 1;       // 2^lg
			for(var lg:int = 0; lg <= MAXBITS; ++lg)        // for each slice
			{
				var lc:int = 0;       // counter
				var lim:int = ttlg;
				if (lim > this._sizeArray)
				{
					lim = this._sizeArray;        // adjust upper limit
					if (i > lim)
					{
						break;        // no more elements to count
					}
				}
				// count elements in range (2^(lg-1), 2^lg]
				for (; i <= lim; ++i)
				{
					if (this._array[i - 1] != Lua.NIL)
					{
						++lc;
					}
				}
				nums[lg] += lc;
				ause += lc;
				ttlg *= 2;
			}
			return ause;
		}

		private function numusehash(nums:Array/*int[] */, pnasize:Array/*int[] */):int
		{
			var totaluse:int = 0;   // total number of elements
			var ause:int = 0;       // summation of nums
			var e:Enumeration;
			e = super.keys();
			while (e.hasMoreElements())
			{
				var o:Object = e. nextElement();
				ause += countint(o, nums);
				++totaluse;
			}
			pnasize[0] += ause;
			return totaluse;
		}

		/**
		 * @param nasize  (new) size of array part
		 */
		private function resize(nasize:int):void
		{
			if (nasize == this._sizeArray)
			{
				return;
			}
			var newarray:Array = new Array(nasize); //Object[] 
			if (nasize > this._sizeArray)     // array part must grow?
			{
				// The new array slots, from sizeArray to nasize-1, must
				// be filled with their values from the hash part.
				// There are two strategies:
				// Iterate over the new array slots, and look up each index in the
				// hash part to see if it has a value; or,
				// Iterate over the hash part and see if each key belongs in the
				// array part.
				// For now we choose the first algorithm.
				// :todo: consider using second algorithm, possibly dynamically.
				SystemUtil.arraycopy(this._array, 0, newarray, 0, this._array.length);
				for (var i:int = this._array.length; i < nasize; ++i)
				{
					var key:Object = new Number(i+1);
					var v:Object = super.remove(key);
					if (v == null)
					{
						v = Lua.NIL;
					}
					newarray[i] = v;
				}
			}
			if (nasize < this._sizeArray)     // array part must shrink?
			{
			  // move elements from array slots nasize to sizeArray-1 to the
			  // hash part.
				for (i = nasize; i < this._sizeArray; ++i)
				{
					if (this._array[i] != Lua.NIL)
					{
						key = new Number(i+1);
						super.put(key, this._array[i]); //TODO:
					}
				}
				SystemUtil.arraycopy(this._array, 0, newarray, 0, newarray.length);
			}
			this._array = newarray;
			this._sizeArray = this._array.length;
		}

		override public function rehash():void
		{
			var oldinrehash:Boolean = this._inrehash;
			this._inrehash = true;
			if (!oldinrehash)
			{
				var nasize:Array = new Array(1); //int[] 
				var nums:Array = new Array(MAXBITS + 1); //int[] 
				nasize[0] = numusearray(nums);      // count keys in array part
				var totaluse:int = nasize[0];
				totaluse += numusehash(nums, nasize);
				var na:int = computesizes(nums, nasize);
			
				resize(nasize[0]);
			}
			super.rehash();
			this._inrehash = oldinrehash;
		}

		/**
		 * Getter for metatable member.
		 * @return  The metatable.
		 */
		public function get metatable():LuaTable
		{
			return _metatable;
		}
		
		/**
		 * Setter for metatable member.
		 * @param metatable  The metatable.
		 */
		// :todo: Support metatable's __gc and __mode keys appropriately.
		//        This involves detecting when those keys are present in the
		//        metatable, and changing all the entries in the Hashtable
		//        to be instance of java.lang.Ref as appropriate.
		public function setMetatable(metatable:LuaTable):void
		{
			this._metatable = metatable;
			return;
		}

		/**
		 * Supports Lua's length (#) operator.  More or less equivalent to
		 * luaH_getn and unbound_search in ltable.c.
		 */
		public function getn():int
		{
			var j:int = this._sizeArray;
			if (j > 0 && this._array[j - 1] == Lua.NIL)
			{
				// there is a boundary in the array part: (binary) search for it
				var i2:int = 0;
				while (j - i2 > 1)
				{
					var m:int = (i2+j)/2;
					if (this._array[m - 1] == Lua.NIL)
					{
						j = m;
					}
					else
					{
						i2 = m;
					}
				}
				return i2;
			}

			// unbound_search

			var i:int = 0;
			j = 1;
			// Find 'i' and 'j' such that i is present and j is not.
			while (this.getnum(j) != Lua.NIL)
			{
				i = j;
				j *= 2;
				if (j < 0)        // overflow
				{
					// Pathological case.  Linear search.
					i = 1;
					while (this.getnum(i) != Lua.NIL)
					{
						++i;
					}
					return i-1;
				}
			}
			// binary search between i and j
			while (j - i > 1)
			{
				var m2:int = (i+j)/2;
				if (this.getnum(m2) == Lua.NIL)
				{
					j = m2;
				}
				else
				{
					i = m2;
				}
			}
			return i;
		}

		/**
		 * Like {@link java.util.Hashtable#get}.  Ensures that indexes
		 * with no value return {@link Lua#NIL}.  In order to get the correct
		 * behaviour for <code>t[nil]</code>, this code assumes that Lua.NIL
		 * is non-<code>null</code>.
		 */
		public function getlua(key:Object):Object
		{
			if (key is Number)
			{
				var d:Number = key as Number;
				if (d <= this._sizeArray && d >=1)
				{
					var i:int = d as int;
					if (i == d)
					{
						return this._array[i - 1];
					}
				}
			}
			var r:Object = super._get(key); //TODO:get
			if (r == null)
			{
				r = Lua.NIL;
			}
			return r;
		}

		/**
		 * Like {@link #getlua(Object)} but the result is written into
		 * the <var>value</var> {@link Slot}.
		 */
		public function __getlua(key:Slot, value:Slot):void
		{
			if (key.r == Lua.NUMBER)
			{
				var d:Number = key.d;
				if (d <= this._sizeArray && d >= 1)
				{
					var i:int = d as int;
					if (i == d)
					{
						value.setObject(this._array[i-1]);
						return;
					}
				}
			}
			var r:Object = super._get(key.asObject()); //TODO:
			if (r == null)
			{
				r = Lua.NIL;
			}
			value.setObject(r);
		}

		/** Like get for numeric (integer) keys. */
		public function getnum(k:int):Object 
		{
			if (k <= this._sizeArray && k >= 1)
			{
				return this._array[k-1];
			}
			var r:Object = super._get(new Number(k)); //TODO:get
			if (r == null)
			{
				return Lua.NIL;
			}
			return r;
		}

		/**
		 * Like {@link java.util.Hashtable#put} but enables Lua's semantics
		 * for <code>nil</code>;
		 * In particular that <code>x = nil</nil>
		 * deletes <code>x</code>.
		 * And also that <code>t[nil]</code> raises an error.
		 * Generally, users of Jill should be using
		 * {@link Lua#setTable} instead of this.
		 * @param key key.
		 * @param value value.
		 */
		public function putluaObj(L:Lua, key:Object, value:Object):void
		{
			var d:Number = 0.0;
			var i:int = int.MAX_VALUE; //TODO:

			if (key == Lua.NIL)
			{
				L.gRunerror("table index is nil");
			}
			if (key is Number)
			{
				d = key as Number;
				var j:int = d as int;

				if (j == d && j >= 1)
				{
					i = j; // will cause additional check for array part later if
						   // the array part check fails now.
					if (i <= this._sizeArray)
					{
						this._array[i - 1] = value;
						return;
					}
				}
				if (isNaN(d))
				{
					L.gRunerror("table index is NaN");
				}
			}
			// :todo: Consider checking key for NaN (PUC-Rio does)
			if (value == Lua.NIL)
			{
				remove(key);
				return;
			}
			super.put(key, value); //TODO:
			// This check is necessary because sometimes the call to super.put
			// can rehash and the new (k,v) pair should be in the array part
			// after the rehash, but is still in the hash part.
			if (i <= this._sizeArray)
			{
				remove(key);
				this._array[i-1] = value;
			}
		}

		public function putluaSlot(L:Lua, key:Slot, value:Object):void
		{
			var i:int = int.MAX_VALUE; //TODO:
			
			if (key.r == Lua.NUMBER)
			{
				var j:int = key.d as int;
				if (j == key.d && j >= 1)
				{
					i = j;
					if (i <= this._sizeArray)
					{
						this._array[i-1] = value;
						return;
					}
				}
				if (isNaN(key.d))
				{
					L.gRunerror("table index is NaN");
				}
			}
			var k:Object = key.asObject();
			// :todo: consider some sort of tail merge with the other putlua
			if (value == Lua.NIL)
			{
				remove(k);
				return;
			}
			super.put(k, value); //TODO:
			if (i <= this._sizeArray)
			{
				remove(k);
				this._array[i - 1] = value;
			}
		}

		/**
		 * Like put for numeric (integer) keys.
		 */
		public function putnum(k:int, v:Object):void
		{
			if (k <= this._sizeArray && k >= 1)
			{
				this._array[k-1] = v;
				return;
			}	
			// The key can never be NIL so putlua will never notice that its L
			// argument is null.
			// :todo: optimisation to avoid putlua checking for array part again
			putluaObj(null, new Number(k), v);
		}

		/**
		 * Do not use, implementation exists only to generate deprecated
		 * warning.
		 * @deprecated Use getlua instead.
		 */
		override public function _get(key:Object):Object
		{
			throw new Error("IllegalArgumentException");
		}

		override public function keys():Enumeration
		{
			return new Enum(this, super.keys());
		}
			
		/**
		 * Do not use, implementation exists only to generate deprecated
		 * warning.
		 * @deprecated Use putlua instead.
		 */
		override public function put(key:Object, value:Object):Object
		{
			throw new Error("IllegalArgumentException");
		}
		  
		/**
		 * Used by oLog2.  DO NOT MODIFY.
		 */
		private static const LOG2:Array =  //final byte[]
		[
			0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
			6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
		];

		/**
		 * Equivalent to luaO_log2.
		 */
		private static function oLog2(x:int):int
		{
			//# assert x >= 0
			
			var l:int = -1;
			while (x >= 256)
			{
				l += 8;
				x >>>= 8;
			}
			return l + LOG2[x];
		}
		
		private static function ceillog2(x:int):int
		{
			return oLog2(x - 1) + 1;
		}
		
		//新增
		public function get array():Array
		{
			return this._array;
		}
		//新增
		public function get sizeArray():int
		{
			return this._sizeArray;
		}
	}
}
