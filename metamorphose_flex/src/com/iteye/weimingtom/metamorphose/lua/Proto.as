/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/Proto.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.IllegalArgumentException;
	import com.iteye.weimingtom.metamorphose.java.NullPointerException;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;
	
	/**
	 * Models a function prototype.  This class is internal to Jill and
	 * should not be used by clients.  This is the analogue of the PUC-Rio
	 * type <code>Proto</code>, hence the name.
	 * A function prototype represents the constant part of a function, that
	 * is, a function without closures (upvalues) and without an
	 * environment.  It's a handle for a block of VM instructions and
	 * ancillary constants.
	 *
	 * For convenience some private arrays are exposed.  Modifying these
	 * arrays is punishable by death. (Java has no convenient constant
	 * array datatype)
	 */
	public final class Proto
	{
		private static const D:Boolean = false; 
		
		/** Interned 0-element array. */
		private static var ZERO_INT_ARRAY:Array = new Array(); /*int[] = new int[0]*/
		private static var ZERO_LOCVAR_ARRAY:Array = new Array(); /*LocVar[]  = new LocVar[0]*/
		private static var ZERO_CONSTANT_ARRAY:Array = new Array();//final Slot[] ZERO_CONSTANT_ARRAY = new Slot[0];
		private static var ZERO_PROTO_ARRAY:Array = new Array();//final Proto[] ZERO_PROTO_ARRAY = new Proto[0];
		private static var ZERO_STRING_ARRAY:Array = new Array();//final String[] ZERO_STRING_ARRAY = new String[0];
  
		
		// Generally the fields are named following the PUC-Rio implementation
		// and so are unusually terse.
		/** Array of constants. */
		private var _k:Array; //Slot[] 
		private var _sizek:int;
		/** Array of VM instructions. */
		private var _code:Array; //int[] 
		private var _sizecode:int;
		/** Array of Proto objects. */
		private var _p:Array; //Proto[] 
		private var _sizep:int;
		/**
		 * Number of upvalues used by this prototype (and so by all the
		 * functions created from this Proto).
		 */
		private var _nups:int;
		/**
		 * Number of formal parameters used by this prototype, and so the
		 * number of argument received by a function created from this Proto.
		 * In a function defined to be variadic then this is the number of
		 * fixed parameters, the number appearing before '...' in the parameter
		 * list.
		 */
		private var _numparams:int;
		/**
		 * <code>true</code> if and only if the function is variadic, that is,
		 * defined with '...' in its parameter list.
		 */
		private var _isVararg:Boolean;
		private var _maxstacksize:int;
		// Debug info
		/** Map from PC to line number. */
		private var _lineinfo:Array; //int[]
		private var _sizelineinfo:int;
		private var _locvars:Array; //LocVar[] 
		private var _sizelocvars:int ;
		private var _upvalues:Array; //String[] 
		private var _sizeupvalues:int; 
		private var _source:String;
		private var _linedefined:int;
		private var _lastlinedefined:int;

		//TODO:
		public function Proto()
		{
			
		}
		
		/**
		 * Proto synthesized by {@link Loader}.
		 * All the arrays that are passed to the constructor are
		 * referenced by the instance.  Avoid unintentional sharing.  All
		 * arrays must be non-null and all int parameters must not be
		 * negative.  Generally, this constructor is used by {@link Loader}
		 * since that has all the relevant arrays already constructed (as
		 * opposed to the compiler).
		 * @param constant   array of constants.
		 * @param code       array of VM instructions.
		 * @param nups       number of upvalues (used by this function).
		 * @param numparams  number of fixed formal parameters.
		 * @param isVararg   whether '...' is used.
		 * @param maxstacksize  number of stack slots required when invoking.
		 * @throws NullPointerException if any array arguments are null.
		 * @throws IllegalArgumentException if nups or numparams is negative.
		 */
		public function init1(constant:Array, //Slot[] 
			code:Array, //int[] 
			proto:Array, //Proto[] 
			nups:int,
			numparams:int,
			isVararg:Boolean,
			maxstacksize:int):void 
		{
			if (null == constant || null == code || null == proto)
			{
				throw new NullPointerException();
			}
			if (nups < 0 || numparams < 0 || maxstacksize < 0)
			{
				throw new IllegalArgumentException();
			}
			this._k = constant; 
			this._sizek = this._k.length ;
			this._code = code;
			this._sizecode = this._code.length ;
			this._p = proto; 
			this._sizep = proto.length ;
			this._nups = nups;
			this._numparams = numparams;
			this.isVararg = isVararg;
			this._maxstacksize = maxstacksize;
		}
		
		/**
		 * Blank Proto in preparation for compilation.
		 * 废弃？
		 */
		public function init2(source:String, maxstacksize:int):void
		{
			this._maxstacksize = maxstacksize;
			// maxstacksize = 2;   // register 0/1 are always valid.
			// :todo: Consider removing size* members
			this._source = source;
			this._k = ZERO_CONSTANT_ARRAY;      
			this._sizek = 0 ;
			this._code = ZERO_INT_ARRAY;        
			this._sizecode = 0 ;
			this._p = ZERO_PROTO_ARRAY;         
			this._sizep = 0;
			this._lineinfo = ZERO_INT_ARRAY;    
			this._sizelineinfo = 0;
			this._locvars = ZERO_LOCVAR_ARRAY;  
			this._sizelocvars = 0 ;
			this._upvalues = ZERO_STRING_ARRAY; 
			this._sizeupvalues = 0;
		}


		/**
		 * Augment with debug info.  All the arguments are referenced by the
		 * instance after the method has returned, so try not to share them.
		 */
		public function debug(lineinfoArg:Array, //int[] 
			locvarsArg:Array, //LocVar[] 
			upvaluesArg:Array):void //String[] 
		{
			this._lineinfo = lineinfoArg;  
			this._sizelineinfo = this._lineinfo.length;
			this._locvars = locvarsArg;    
			this._sizelocvars = this._locvars.length;
			this._upvalues = upvaluesArg;  
			this._sizeupvalues = this._upvalues.length;
		}

		/** Gets source. */
		public function get source():String
		{
			return this._source;
		}

		/** Setter for source. */
		public function set source(source:String):void
		{
			this._source = source;
		}

		public function get linedefined():int
		{
			return this._linedefined;
		}
	  
		public function set linedefined(linedefined:int):void
		{
			this._linedefined = linedefined;
		}

		public function get lastlinedefined():int
		{
			return _lastlinedefined;
		}
	  
		public function set lastlinedefined(lastlinedefined:int):void
		{
			this._lastlinedefined = lastlinedefined;
		}

		/** Gets Number of Upvalues */
		public function get nups():int
		{
			return this._nups;
		}

		public function set nups(nups:int):void
		{
			this._nups = nups;
		}
		
		/** Number of Parameters. */
		public function get numparams():int
		{
			return this._numparams;
		}

		public function set numparams(numparams:int):void
		{
			this._numparams = numparams;
		}
		
		/** Maximum Stack Size. */
		public function get maxstacksize():int
		{
			return this._maxstacksize;
		}

		/** Setter for maximum stack size. */
		public function set maxstacksize(m:int):void
		{
			this._maxstacksize = m;
		}

		/** Instruction block (do not modify). */
		public function get code():Array //int[] 
		{
			return this._code;
		}

		/** Append instruction. */
		public function codeAppend(L:Lua, pc:int, instruction:int, line:int):void
		{
			if (D) 
			{
				trace("pc:" + pc + 
					", instruction:" + instruction + 
					", line:" + line + 
					", lineinfo.length:" + lineinfo.length);
			}
			
			ensureCode(L, pc);
			this._code[pc] = instruction;

			if (pc >= this._lineinfo.length)
			{
				var newLineinfo:Array = new Array(this._lineinfo.length * 2 + 1); //int[]
				SystemUtil.arraycopy(this._lineinfo, 0, newLineinfo, 0, this._lineinfo.length);
				this._lineinfo = newLineinfo;
			}
			this._lineinfo[pc] = line;
		}

		public function ensureLocvars(L:Lua, atleast:int, limit:int):void
		{
			if (atleast + 1 > this._sizelocvars)
			{
				var newsize:int = atleast * 2 + 1 ;
				if (newsize > limit)
					newsize = limit ;
				if (atleast + 1 > newsize)
					L.gRunerror("too many local variables") ;
				var newlocvars:Array = new Array(newsize); //LocVar []
				SystemUtil.arraycopy(locvars, 0, newlocvars, 0, this._sizelocvars) ;
				for (var i:int = this._sizelocvars ; i < newsize ; i++)
					newlocvars[i] = new LocVar();
				this._locvars = newlocvars ;
				this._sizelocvars = newsize ;
			}
		}

		public function ensureProtos(L:Lua, atleast:int):void
		{
			if (atleast + 1 > this._sizep)
			{
				var newsize:int = atleast * 2 + 1 ;
				if (newsize > Lua.MAXARG_Bx)
					newsize = Lua.MAXARG_Bx ;
				if (atleast + 1 > newsize)
					L.gRunerror("constant table overflow") ;
				var newprotos:Array = new Array(newsize) ; //Proto [] 
				SystemUtil.arraycopy(this._p, 0, newprotos, 0, this._sizep) ;
				this._p = newprotos ;
				this._sizep = newsize ;
			}
		}

		public function ensureUpvals(L:Lua, atleast:int):void
		{
			if (atleast + 1 > this._sizeupvalues)
			{
				var newsize:int = atleast * 2 + 1;
				if (atleast + 1 > newsize)
					L.gRunerror("upvalues overflow") ;
				var newupvalues:Array = new Array(newsize); //String []
				SystemUtil.arraycopy(this._upvalues, 0, newupvalues, 0, this._sizeupvalues) ;
				this._upvalues = newupvalues ;
				this._sizeupvalues = newsize ;
			}
		}

		public function ensureCode(L:Lua, atleast:int):void
		{
			if (atleast + 1 > this._sizecode)
			{
				var newsize:int = atleast * 2 + 1;
				if (atleast + 1 > newsize)
					L.gRunerror("code overflow") ;
				var newcode:Array = new Array(newsize); //int [] 
				SystemUtil.arraycopy(this._code, 0, newcode, 0, this._sizecode) ;
				this._code = newcode ;
				this._sizecode = newsize ;
			}
		}

		/** Set lineinfo record. */
		public function setLineinfo(pc:int, line:int):void
		{
			this._lineinfo[pc] = line;
		}

		/** Get linenumber corresponding to pc, or 0 if no info. */
		public function getline(pc:int):int
		{
			if (this._lineinfo.length == 0)
			{
				return 0;
			}
			return this._lineinfo[pc];
		}

		/** Array of inner protos (do not modify). */
		public function get proto():Array //Proto[] 
		{
			return this._p;
		}

		/** Constant array (do not modify). */
		public function get constant():Array //Slot[] 
		{
			return this._k;
		}

		/** Append constant. */
		public function constantAppend(idx:int, o:Object):void
		{
			if (idx >= this._k.length)
			{
				var newK:Array = new Array(this._k.length * 2 + 1); //Slot[]
				SystemUtil.arraycopy(this._k, 0, newK, 0, this._k.length);
				this._k = newK;
			}
			this._k[idx] = new Slot();
			(this._k[idx] as Slot).init2(o);
		}

		/** Predicate for whether function uses ... in its parameter list. */
		public function get isVararg():Boolean
		{
			return _isVararg;
		}

		/** "Setter" for isVararg.  Sets it to true. */
		public function set isVararg(isVararg:Boolean):void
		{
			_isVararg = true;
		}

		/** LocVar array (do not modify). */
		public function get locvars():Array //LocVar[] 
		{
			return _locvars;
		}

		// All the trim functions, below, check for the redundant case of
		// trimming to the length that they already are.  Because they are
		// initially allocated as interned zero-length arrays this also means
		// that no unnecesary zero-length array objects are allocated.

		/**
		 * Trim an int array to specified size.
		 * @return the trimmed array.
		 */
		private function trimInt(old:Array/*int[] */, n:int):Array //int[]
		{
			if (n == old.length)
			{
				return old;
			}
			var newArray:Array = new Array(n); //int[] 
			SystemUtil.arraycopy(old, 0, newArray, 0, n);
			return newArray;
		}

		/** Trim code array to specified size. */
		public function closeCode(n:int):void
		{
			this._code = trimInt(this._code, n);
			this._sizecode = this._code.length ;
		}

		/** Trim lineinfo array to specified size. */
		public function closeLineinfo(n:int):void
		{
			this._lineinfo = trimInt(this._lineinfo, n);
			this._sizelineinfo = n;
		}

		/** Trim k (constant) array to specified size. */
		public function closeK(n:int):void
		{
			if (this._k.length > n)
			{
				var newArray:Array = new Array(n); //Slot[] 
				SystemUtil.arraycopy(this._k, 0, newArray, 0, n);
				this._k = newArray;
			}
			this._sizek = n;
			return;
		}

		/** Trim p (proto) array to specified size. */
		public function closeP(n:int):void
		{
			if (n == this._p.length)
			{
				return;
			}
			var newArray:Array = new Array(n); //Proto[] 
			SystemUtil.arraycopy(this._p, 0, newArray, 0, n);
			this._p = newArray;
			this._sizep = n ;
		}

		/** Trim locvar array to specified size. */
		public function closeLocvars(n:int):void
		{
			if (n == locvars.length)
			{
				return;
			}
			var newArray:Array = new Array(n); //LocVar[] 
			SystemUtil.arraycopy(locvars, 0, newArray, 0, n);
			this._locvars = newArray;
			this._sizelocvars = n;
		}

		/** Trim upvalues array to size <var>nups</var>. */
		public function closeUpvalues():void
		{
			if (nups == this._upvalues.length)
			{
				return;
			}
			var newArray:Array = new Array(nups); //String[] 
			SystemUtil.arraycopy(this._upvalues, 0, newArray, 0, nups);
			this._upvalues = newArray;
			this._sizeupvalues = nups;
		}
		
		//新增
		public function get k():Array
		{
			return this._k;
		}
		
		//新增
		public function get sizek():int
		{
			return this._sizek;
		}
		
		//新增
		public function get sizecode():int
		{
			return this._sizecode;
		}

		//新增
		public function get p():Array
		{
			return this._p;
		}
		//新增
		public function get sizep():int
		{
			return this._sizep;
		}
		//新增
		public function get lineinfo():Array
		{
			return this._lineinfo;
		}	
		//新增
		public function get sizelineinfo():int
		{
			return this._sizelineinfo;
		}	
		//新增
		public function get sizelocvars():int
		{
			return this._sizelocvars;
		}	
		//新增
		public function get upvalues():Array
		{
			return this._upvalues;
		}
		//新增
		public function get sizeupvalues():int
		{
			return this._sizeupvalues;
		}
		
	}
}