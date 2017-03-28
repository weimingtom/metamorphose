/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/FuncState.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.Hashtable;
	import com.iteye.weimingtom.metamorphose.java.IllegalArgumentException;
	
	/**
	 * Used to model a function during compilation.  Code generation uses
	 * this structure extensively.  Most of the PUC-Rio functions from
	 * lcode.c have moved into this class, alongwith a few functions from
	 * lparser.c
	 */
	public final class FuncState
	{
		/** See NO_JUMP in lcode.h. */
		public static const NO_JUMP:int = -1;

		/** Proto object for this function. */
		private var _f:Proto;

		/**
		* Table to find (and reuse) elements in <var>f.k</var>.  Maps from
		* Object (a constant Lua value) to an index into <var>f.k</var>.
		*/
		private var _h:Hashtable = new Hashtable();

		/** Enclosing function. */
		private var _prev:FuncState;

		/** Lexical state. */
		private var _ls:Syntax;

		/** Lua state. */
		private var _L:Lua;

		/** chain of current blocks */
		private var _bl:BlockCnt;  // = null;

		/** next position to code. */
		private var _pc:int;       // = 0;

		/** pc of last jump target. */
		private var _lasttarget:int = -1;

		/** List of pending jumps to <var>pc</var>. */
		private var _jpc:int = NO_JUMP;

		/** First free register. */
		private var _freereg:int;  // = 0;

		/** number of elements in <var>k</var>. */
		private var _nk:int;       // = 0;

		/** number of elements in <var>p</var>. */
		private var _np:int;       // = 0;

		/** number of elements in <var>locvars</var>. */
		private var _nlocvars:int;       // = 0;

		/** number of active local variables. */
		private var _nactvar:int;        // = 0;

		/** upvalues as 8-bit k and 8-bit info */
		private var _upvalues:Array = new Array(Lua.MAXUPVALUES); //int [] 

		/** declared-variable stack. */
		private var _actvar:Array = new Array(Lua.MAXVARS); //short[] 

		/**
		 * Constructor.  Much of this is taken from <code>open_func</code> in
		 * <code>lparser.c</code>.
		 */
		public function FuncState(ls:Syntax)
		{
			this._f = new Proto();
			this._f.init2(ls.source, 2); // default value for maxstacksize=2
			this._L = ls.L ;
			this._ls = ls;
			//    prev = ls.linkfs(this);
		}
	
		/** Equivalent to <code>close_func</code> from <code>lparser.c</code>. */
		public function close():void
		{
			this._f.closeCode(this._pc);
			this._f.closeLineinfo(this._pc);
			this._f.closeK(this._nk);
			this._f.closeP(this._np);
			this._f.closeLocvars(this._nlocvars);
			this._f.closeUpvalues();
			var checks:Boolean = this._L.gCheckcode(this._f);
			//# assert checks
			//# assert bl == null
		}

		/** Equivalent to getlocvar from lparser.c.
		* Accesses <code>LocVar</code>s of the {@link Proto}.
		*/
		public function getlocvar(idx:int):LocVar
		{
			return this._f.locvars[this._actvar[idx]];
		}


		// Functions from lcode.c

		/** Equivalent to luaK_checkstack. */
		public function kCheckstack(n:int):void 
		{
			var newstack:int = this._freereg + n;
			if (newstack > this._f.maxstacksize)
			{
				if (newstack >= Lua.MAXSTACK)
				{
					this._ls.xSyntaxerror("function or expression too complex");
				}
				this._f.maxstacksize = newstack;
			}
		}

		/** Equivalent to luaK_code. */
		public function kCode(i:int, line:int):int
		{
			dischargejpc();
			// Put new instruction in code array.
			this._f.codeAppend(this._L, this._pc, i, line);
			return this._pc++;
		}

		/** Equivalent to luaK_codeABC. */
		public function kCodeABC(o:int, a:int, b:int, c:int):int
		{
			// assert getOpMode(o) == iABC;
			// assert getBMode(o) != OP_ARG_N || b == 0;
			// assert getCMode(o) != OP_ARG_N || c == 0;
			return kCode(Lua.CREATE_ABC(o, a, b, c), this._ls.lastline);
		}

		/** Equivalent to luaK_codeABx. */
		public function kCodeABx(o:int, a:int, bc:int):int
		{
			// assert getOpMode(o) == iABx || getOpMode(o) == iAsBx);
			// assert getCMode(o) == OP_ARG_N);
			return kCode(Lua.CREATE_ABx(o, a, bc), this._ls.lastline);
		}

		/** Equivalent to luaK_codeAsBx. */
		public function kCodeAsBx(o:int, a:int, bc:int):int
		{
			return kCodeABx(o, a, bc+Lua.MAXARG_sBx);
		}

		/** Equivalent to luaK_dischargevars. */
		public function kDischargevars(e:Expdesc):void
		{
			switch (e.kind)
			{
				case Expdesc.VLOCAL:
					e.kind = Expdesc.VNONRELOC;
					break;
		  
				case Expdesc.VUPVAL:
					e.reloc(kCodeABC(Lua.OP_GETUPVAL, 0, e.info, 0));
					break;
		  
				case Expdesc.VGLOBAL:
					e.reloc(kCodeABx(Lua.OP_GETGLOBAL, 0, e.info));
					break;
		  
				case Expdesc.VINDEXED:
					__freereg(e.aux); //TODO:
					__freereg(e.info); //TODO:
					e.reloc(kCodeABC(Lua.OP_GETTABLE, 0, e.info, e.aux));
					break;
		  
				case Expdesc.VVARARG:
				case Expdesc.VCALL:
					kSetoneret(e);
					break;
		  
				default:
					break;  // there is one value available (somewhere)
			}
		}

		/** Equivalent to luaK_exp2anyreg. */
		public function kExp2anyreg(e:Expdesc):int
		{
			kDischargevars(e);
			if (e.k == Expdesc.VNONRELOC)
			{
				if (!e.hasjumps())
				{
					return e.info;
				}
				if (e.info >= this._nactvar)          // reg is not a local?
				{
					exp2reg(e, e.info);   // put value on it
					return e.info;
				}
			}
			kExp2nextreg(e);    // default
			return e.info;
		}

		/** Equivalent to luaK_exp2nextreg. */
		public function kExp2nextreg(e:Expdesc):void 
		{
			kDischargevars(e);
			freeexp(e);
			kReserveregs(1);
			exp2reg(e, this._freereg - 1);
		}

		/** Equivalent to luaK_fixline. */
		public function kFixline(line:int):void 
		{
			this._f.setLineinfo(this._pc - 1, line);
		}

		/** Equivalent to luaK_infix. */
		public function kInfix(op:int, v:Expdesc):void
		{
			switch (op)
			{
				case Syntax.OPR_AND:
					kGoiftrue(v);
					break;
				
				case Syntax.OPR_OR:
					kGoiffalse(v);
					break;
		
				case Syntax.OPR_CONCAT:
					kExp2nextreg(v);  /* operand must be on the `stack' */
					break;
		
				default:
					if (!isnumeral(v))
						kExp2RK(v);
					break;
			}
		}
		
		private function isnumeral(e:Expdesc):Boolean
		{
			return e.k == Expdesc.VKNUM &&
				e.t == NO_JUMP &&
				e.f == NO_JUMP;
		}

		/** Equivalent to luaK_nil. */
		public function kNil(from:int, n:int):void 
		{
			var previous:int;
			if (this._pc > this._lasttarget)   /* no jumps to current position? */
			{
				if (this._pc == 0)  /* function start? */
					return;  /* positions are already clean */
				previous = this._pc - 1 ;
				var instr:int = this._f.code[previous] ;
				if (Lua.OPCODE(instr) == Lua.OP_LOADNIL)
				{
					var pfrom:int = Lua.ARGA(instr);
					var pto:int = Lua.ARGB(instr);
					if (pfrom <= from && from <= pto+1)  /* can connect both? */
					{
						if (from+n-1 > pto)
							this._f.code[previous] = Lua.SETARG_B(instr, from+n-1);
						return;
					}
				}
			}
			kCodeABC(Lua.OP_LOADNIL, from, from + n - 1, 0);
		}

		/** Equivalent to luaK_numberK. */
		public function kNumberK(r:Number):int 
		{
			return addk(Lua.valueOfNumber(r)); //TODO:L->Lua
		}

		/** Equivalent to luaK_posfix. */
		public function kPosfix(op:int, e1:Expdesc, e2:Expdesc):void
		{
			switch (op)
			{
				case Syntax.OPR_AND:
					/* list must be closed */
					//# assert e1.t == NO_JUMP
					kDischargevars(e2);
					e2.f = kConcat(e2.f, e1.f);
					e1.copy(e2); //TODO:
					break;

				case Syntax.OPR_OR:
					/* list must be closed */
					//# assert e1.f == NO_JUMP
					kDischargevars(e2);
					e2.t = kConcat(e2.t, e1.t);
					e1.copy(e2); //TODO:
					break;

				case Syntax.OPR_CONCAT:
					kExp2val(e2);
					if (e2.k == Expdesc.VRELOCABLE && Lua.OPCODE(getcode(e2)) == Lua.OP_CONCAT)
					{
						//# assert e1.info == Lua.ARGB(getcode(e2))-1
						freeexp(e1);
						setcode(e2, Lua.SETARG_B(getcode(e2), e1.info));
						e1.k = e2.k;
						e1.info = e2.info;
					}
					else
					{
						kExp2nextreg(e2);  /* operand must be on the 'stack' */
						codearith(Lua.OP_CONCAT, e1, e2);
					}
					break;
			
				case Syntax.OPR_ADD: 
					codearith(Lua.OP_ADD, e1, e2); 
					break;
			  
				case Syntax.OPR_SUB: 
					codearith(Lua.OP_SUB, e1, e2); 
					break;
			  
				case Syntax.OPR_MUL: 
					codearith(Lua.OP_MUL, e1, e2); 
					break;
			  
				case Syntax.OPR_DIV: 
					codearith(Lua.OP_DIV, e1, e2); 
					break;
			  
				case Syntax.OPR_MOD: 
					codearith(Lua.OP_MOD, e1, e2); 
					break;
			  
				case Syntax.OPR_POW: 
					codearith(Lua.OP_POW, e1, e2); 
					break;
			  
				case Syntax.OPR_EQ: 
					codecomp(Lua.OP_EQ, true,  e1, e2); 
					break;
			  
				case Syntax.OPR_NE: 
					codecomp(Lua.OP_EQ, false, e1, e2); 
					break;
			  
				case Syntax.OPR_LT: 
					codecomp(Lua.OP_LT, true,  e1, e2); 
					break;
			  
				case Syntax.OPR_LE: 
					codecomp(Lua.OP_LE, true,  e1, e2); 
					break;
			  
				case Syntax.OPR_GT: 
					codecomp(Lua.OP_LT, false, e1, e2); 
					break;
			  
				case Syntax.OPR_GE: 
					codecomp(Lua.OP_LE, false, e1, e2); 
					break;
			  
				default:
					//# assert false
			}
		}
		
		/** Equivalent to luaK_prefix. */
		public function kPrefix(op:int, e:Expdesc):void
		{
			var e2:Expdesc = new Expdesc();// TODO:
			e2.init(Expdesc.VKNUM, 0);
			switch (op)
			{
				case Syntax.OPR_MINUS:
					if (e.kind == Expdesc.VK)
					{
						kExp2anyreg(e);
					}
					codearith(Lua.OP_UNM, e, e2);
					break;
				  
				case Syntax.OPR_NOT:
					codenot(e);
					break;
				
				case Syntax.OPR_LEN:
					kExp2anyreg(e);
					codearith(Lua.OP_LEN, e, e2);
					break;
				
				default:
					throw new IllegalArgumentException();
			}
		}

		/** Equivalent to luaK_reserveregs. */
		public function kReserveregs(n:int):void
		{
			kCheckstack(n);
			this._freereg += n;
		}

		/** Equivalent to luaK_ret. */
		public function kRet(first:int, nret:int):void
		{
			kCodeABC(Lua.OP_RETURN, first, nret+1, 0);
		}
		
		/** Equivalent to luaK_setmultret (in lcode.h). */
		public function kSetmultret(e:Expdesc):void
		{
			kSetreturns(e, Lua.MULTRET);
		}
		
		/** Equivalent to luaK_setoneret. */
		public function kSetoneret(e:Expdesc):void
		{
			if (e.kind == Expdesc.VCALL)      // expression is an open function call?
			{
				e.nonreloc(Lua.ARGA(getcode(e)));
			}
			else if (e.kind == Expdesc.VVARARG)
			{
				setargb(e, 2);
				e.kind = Expdesc.VRELOCABLE;
			}
		}
		
		/** Equivalent to luaK_setreturns. */
		public function kSetreturns(e:Expdesc, nresults:int):void
		{
			if (e.kind == Expdesc.VCALL)      // expression is an open function call?
			{
				setargc(e, nresults+1);
			}
			else if (e.kind == Expdesc.VVARARG)
			{
				setargb(e, nresults+1);
				setarga(e, this._freereg);
				kReserveregs(1);
			}
		}

		/** Equivalent to luaK_stringK. */
		public function kStringK(s:String):int
		{
			return addk(s/*.intern()*/);
		}

		private function addk(o:Object):int
		{
			var hash:Object = o;
			var v:Object = _h._get(hash); //TODO:get
			if (v != null)
			{
				// :todo: assert
				return v as int; //TODO:
			}
			// constant not found; create a new entry
			this._f.constantAppend(this._nk, o);
			this._h.put(hash, new int(this._nk)); //TODO:
			return this._nk++;
		}
		
		private function codearith(op:int, e1:Expdesc, e2:Expdesc):void
		{
			if (constfolding(op, e1, e2))
				return;
			else
			{
				var o1:int = kExp2RK(e1);
				var o2:int = (op != Lua.OP_UNM && op != Lua.OP_LEN) ? kExp2RK(e2) : 0;
				freeexp(e2);
				freeexp(e1);
				e1.info = kCodeABC(op, 0, o1, o2);
				e1.k = Expdesc.VRELOCABLE;
			}
		}

		private function constfolding(op:int, e1:Expdesc, e2:Expdesc):Boolean
		{
			var r:Number = 0;
			if (!isnumeral(e1) || !isnumeral(e2))
				return false;
			
			var v1:Number = e1.nval;
			var v2:Number = e2.nval;
			switch (op)
			{
				case Lua.OP_ADD: 
					r = v1 + v2; 
					break;
			  
				case Lua.OP_SUB: 
					r = v1 - v2; 
					break;
			  
				case Lua.OP_MUL: 
					r = v1 * v2; 
					break;
			  
				case Lua.OP_DIV:
					if (v2 == 0.0)
						return false;  /* do not attempt to divide by 0 */
					r = v1 / v2;
					break;
			  
				case Lua.OP_MOD:
					if (v2 == 0.0)
						return false;  /* do not attempt to divide by 0 */
					r = v1 % v2;
					break;
			  
				case Lua.OP_POW: 
					r = Lua.iNumpow(v1, v2);  //TODO:L->Lua
					break;
			  
				case Lua.OP_UNM: 
					r = -v1; 
					break;
			  
				case Lua.OP_LEN: 
					return false;  /* no constant folding for 'len' */
			  
				default:
					//# assert false
					r = 0.0; 
					break;
			}
			if (isNaN(r))
				return false;  /* do not attempt to produce NaN */
			e1.nval = r;
			return true;
		}

		private function codenot(e:Expdesc):void
		{
			kDischargevars(e);
			switch (e.k)
			{
				case Expdesc.VNIL:
				case Expdesc.VFALSE:
					e.k = Expdesc.VTRUE;
					break;

				case Expdesc.VK:
				case Expdesc.VKNUM:
				case Expdesc.VTRUE:
					e.k = Expdesc.VFALSE;
					break;

				case Expdesc.VJMP:
					invertjump(e);
					break;

				case Expdesc.VRELOCABLE:
				case Expdesc.VNONRELOC:
					discharge2anyreg(e);
					freeexp(e);
					e.info = kCodeABC(Lua.OP_NOT, 0, e.info, 0);
					e.k = Expdesc.VRELOCABLE;
					break;

				default:
					//# assert false
					break;
			}
			/* interchange true and false lists */
			{ 
				var temp:int = e.f; 
				e.f = e.t; 
				e.t = temp; 
			}
			removevalues(e.f);
			removevalues(e.t);
		}

		private function removevalues(list:int):void
		{
			for (; list != NO_JUMP; list = getjump(list))
				patchtestreg(list, Lua.NO_REG);
		}
		
		private function dischargejpc():void
		{
			patchlistaux(this._jpc, this._pc, Lua.NO_REG, this._pc);
			this._jpc = NO_JUMP;
		}

		private function discharge2reg(e:Expdesc, reg:int):void
		{
			kDischargevars(e);
			switch (e.k)
			{
				case Expdesc.VNIL:
					kNil(reg, 1);
					break;

				case Expdesc.VFALSE:
				case Expdesc.VTRUE:
					kCodeABC(Lua.OP_LOADBOOL, reg, (e.k == Expdesc.VTRUE ? 1 : 0), 0);
					break;

				case Expdesc.VK:
					kCodeABx(Lua.OP_LOADK, reg, e.info);
					break;

				case Expdesc.VKNUM:
					kCodeABx(Lua.OP_LOADK, reg, kNumberK(e.nval));
					break;

				case Expdesc.VRELOCABLE:
					setarga(e, reg);
					break;

				case Expdesc.VNONRELOC:
					if (reg != e.info)
					{
					  kCodeABC(Lua.OP_MOVE, reg, e.info, 0);
					}
					break;

				case Expdesc.VVOID:
				case Expdesc.VJMP:
					return ;

				default:
					//# assert false
			}
			e.nonreloc(reg);
		}

		private function exp2reg(e:Expdesc, reg:int):void
		{
			discharge2reg(e, reg);
			if (e.k == Expdesc.VJMP)
			{
				e.t = kConcat(e.t, e.info);  /* put this jump in `t' list */
			}
			if (e.hasjumps())
			{
				var p_f:int = NO_JUMP;  /* position of an eventual LOAD false */
				var p_t:int = NO_JUMP;  /* position of an eventual LOAD true */
				if (need_value(e.t) || need_value(e.f))
				{
					var fj:int = (e.k == Expdesc.VJMP) ? NO_JUMP : kJump();
					p_f = code_label(reg, 0, 1);
					p_t = code_label(reg, 1, 0);
					kPatchtohere(fj);
				}
				var finalpos:int = kGetlabel(); /* position after whole expression */
				patchlistaux(e.f, finalpos, reg, p_f);
				patchlistaux(e.t, finalpos, reg, p_t);
			}
			e.init(Expdesc.VNONRELOC, reg);
		}
		
		private function code_label(a:int, b:int, jump:int):int
		{
			kGetlabel();  /* those instructions may be jump targets */
			return kCodeABC(Lua.OP_LOADBOOL, a, b, jump);
		}
		
		/**
		 * check whether list has any jump that do not produce a value
		 * (or produce an inverted value)
		 */
		private function need_value(list:int):Boolean
		{
			for (; list != NO_JUMP; list = getjump(list))
			{
				var i:int = getjumpcontrol(list);
				var instr:int = this._f.code[i] ;
				if (Lua.OPCODE(instr) != Lua.OP_TESTSET)
					return true;
			}
			return false;  /* not found */
		}

		private function freeexp(e:Expdesc):void
		{
			if (e.kind == Expdesc.VNONRELOC)
			{
				__freereg(e.info);
			}
		}
		
		public function set freereg(freereg:int):void
		{
			this._freereg = freereg;
		}

		public function get freereg():int
		{
			return this._freereg;
		}
		
		private function __freereg(reg:int):void
		{
			if (!Lua.ISK(reg) && reg >= this._nactvar)
			{
				--this._freereg;
				// assert reg == freereg;
			}
		}
		
		public function getcode(e:Expdesc):int
		{
			return this._f.code[e.info];
		}

		public function setcode(e:Expdesc, code:int):void
		{
			this._f.code[e.info] = code ;
		}

		/** Equivalent to searchvar from lparser.c */
		public function searchvar(n:String):int
		{
			// caution: descending loop (in emulation of PUC-Rio).
			for (var i:int = this._nactvar - 1; i >= 0; i--)
			{
				if (n == getlocvar(i).varname)
					return i;
			}
			return -1;  // not found
		}

		public function setarga(e:Expdesc, a:int):void
		{
			var at:int = e.info;
			var code:Array = this._f.code; //int[] 
			code[at] = Lua.SETARG_A(code[at] as int, a);
		}

		public function setargb(e:Expdesc, b:int):void
		{
			var at:int = e.info;
			var code:Array = this._f.code; //int[] 
			code[at] = Lua.SETARG_B(code[at] as int, b);
		}

		public function setargc(e:Expdesc, c:int):void
		{
			var at:int = e.info;
			var code:Array = this._f.code; //int[]
			code[at] = Lua.SETARG_C(code[at] as int, c);
		}
		
		/** Equivalent to <code>luaK_getlabel</code>. */
		public function kGetlabel():int
		{
			this._lasttarget = this._pc ;
			return this._pc;
		}

		/**
		* Equivalent to <code>luaK_concat</code>.
		* l1 was an int*, now passing back as result.
		*/
		public function kConcat(l1:int, l2:int):int 
		{
			if (l2 == NO_JUMP)
				return l1;
			else if (l1 == NO_JUMP)
				return l2;
			else
			{
				var list:int = l1;
				var next:int;
				while ((next = getjump(list)) != NO_JUMP)  /* find last element */
					list = next;
				fixjump(list, l2);
				return l1;
			}
		}

		/** Equivalent to <code>luaK_patchlist</code>. */
		public function kPatchlist(list:int, target:int):void
		{
			if (target == this._pc)
				kPatchtohere(list);
			else
			{
				//# assert target < pc
				patchlistaux(list, target, Lua.NO_REG, target);
			}
		}

		private function patchlistaux(list:int, vtarget:int, reg:int,
								dtarget:int):void
		{
			while (list != NO_JUMP)
			{
				var next:int = getjump(list);
				if (patchtestreg(list, reg))
					fixjump(list, vtarget);
				else
					fixjump(list, dtarget);  /* jump to default target */
				list = next;
			}
		}

		private function patchtestreg(node:int, reg:int):Boolean
		{
			var i:int = getjumpcontrol(node);
			var code:Array = this._f.code; //int [] 
			var instr:int = code[i] ;
			if (Lua.OPCODE(instr) != Lua.OP_TESTSET)
				return false;  /* cannot patch other instructions */
			if (reg != Lua.NO_REG && reg != Lua.ARGB(instr))
				code[i] = Lua.SETARG_A(instr, reg);
			else  /* no register to put value or register already has the value */
				code[i] = Lua.CREATE_ABC(Lua.OP_TEST, Lua.ARGB(instr), 0, Lua.ARGC(instr));
		
			return true;
		}

		private function getjumpcontrol(at:int):int
		{
			var code:Array = this._f.code; //int []
			if (at >= 1 && testTMode(Lua.OPCODE(code[at-1])))
				return at - 1;
			else
				return at;
		}

		/*
		** masks for instruction properties. The format is:
		** bits 0-1: op mode
		** bits 2-3: C arg mode
		** bits 4-5: B arg mode
		** bit 6: instruction set register A
		** bit 7: operator is a test
		*/

		/** arg modes */
		private static const OP_ARG_N:int = 0 ;
		private static const OP_ARG_U:int = 1 ;
		private static const OP_ARG_R:int = 2 ;
		private static const OP_ARG_K:int = 3 ;
		
		/** op modes */
		private static const iABC:int = 0 ;
		private static const iABx:int = 1 ;
		private static const iAsBx:int = 2 ;

		public static function opmode(t:int, a:int, b:int, c:int, m:int):int
		{
			return ((t<<7) | (a<<6) | (b<<4) | (c<<2) | m) as int;
		}

		private static var OPMODE:Array = //new byte []
		[
			/*      T  A  B         C         mode                opcode  */
			 opmode(0, 1, OP_ARG_R, OP_ARG_N, iABC)            /* OP_MOVE */
			,opmode(0, 1, OP_ARG_K, OP_ARG_N, iABx)            /* OP_LOADK */
			,opmode(0, 1, OP_ARG_U, OP_ARG_U, iABC)            /* OP_LOADBOOL */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iABC)            /* OP_LOADNIL */
			,opmode(0, 1, OP_ARG_U, OP_ARG_N, iABC)            /* OP_GETUPVAL */
			,opmode(0, 1, OP_ARG_K, OP_ARG_N, iABx)            /* OP_GETGLOBAL */
			,opmode(0, 1, OP_ARG_R, OP_ARG_K, iABC)            /* OP_GETTABLE */
			,opmode(0, 0, OP_ARG_K, OP_ARG_N, iABx)            /* OP_SETGLOBAL */
			,opmode(0, 0, OP_ARG_U, OP_ARG_N, iABC)            /* OP_SETUPVAL */
			,opmode(0, 0, OP_ARG_K, OP_ARG_K, iABC)            /* OP_SETTABLE */
			,opmode(0, 1, OP_ARG_U, OP_ARG_U, iABC)            /* OP_NEWTABLE */
			,opmode(0, 1, OP_ARG_R, OP_ARG_K, iABC)            /* OP_SELF */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_ADD */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_SUB */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_MUL */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_DIV */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_MOD */
			,opmode(0, 1, OP_ARG_K, OP_ARG_K, iABC)            /* OP_POW */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iABC)            /* OP_UNM */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iABC)            /* OP_NOT */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iABC)            /* OP_LEN */
			,opmode(0, 1, OP_ARG_R, OP_ARG_R, iABC)            /* OP_CONCAT */
			,opmode(0, 0, OP_ARG_R, OP_ARG_N, iAsBx)           /* OP_JMP */
			,opmode(1, 0, OP_ARG_K, OP_ARG_K, iABC)            /* OP_EQ */
			,opmode(1, 0, OP_ARG_K, OP_ARG_K, iABC)            /* OP_LT */
			,opmode(1, 0, OP_ARG_K, OP_ARG_K, iABC)            /* OP_LE */
			,opmode(1, 1, OP_ARG_R, OP_ARG_U, iABC)            /* OP_TEST */
			,opmode(1, 1, OP_ARG_R, OP_ARG_U, iABC)            /* OP_TESTSET */
			,opmode(0, 1, OP_ARG_U, OP_ARG_U, iABC)            /* OP_CALL */
			,opmode(0, 1, OP_ARG_U, OP_ARG_U, iABC)            /* OP_TAILCALL */
			,opmode(0, 0, OP_ARG_U, OP_ARG_N, iABC)            /* OP_RETURN */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iAsBx)           /* OP_FORLOOP */
			,opmode(0, 1, OP_ARG_R, OP_ARG_N, iAsBx)           /* OP_FORPREP */
			,opmode(1, 0, OP_ARG_N, OP_ARG_U, iABC)            /* OP_TFORLOOP */
			,opmode(0, 0, OP_ARG_U, OP_ARG_U, iABC)            /* OP_SETLIST */
			,opmode(0, 0, OP_ARG_N, OP_ARG_N, iABC)            /* OP_CLOSE */
			,opmode(0, 1, OP_ARG_U, OP_ARG_N, iABx)            /* OP_CLOSURE */
			,opmode(0, 1, OP_ARG_U, OP_ARG_N, iABC)            /* OP_VARARG */
		];

		private function getOpMode(m:int):int
		{
			return OPMODE[m] & 3 ;
		}
		
		private function testAMode(m:int):Boolean
		{
			return (OPMODE[m] & (1<<6)) != 0 ;
		}
		
		private function testTMode(m:int):Boolean 
		{
			return (OPMODE[m] & (1<<7)) != 0 ;
		}

		/** Equivalent to <code>luaK_patchtohere</code>. */
		public function kPatchtohere(list:int):void
		{
			kGetlabel();
			this._jpc = kConcat(this._jpc, list);
		}

		private function fixjump(at:int, dest:int):void
		{
			var jmp:int = this._f.code[at];
			var offset:int = dest-(at+1);
			//# assert dest != NO_JUMP
			if (Math.abs(offset) > Lua.MAXARG_sBx)
				this._ls.xSyntaxerror("control structure too long");
			this._f.code[at] = Lua.SETARG_sBx(jmp, offset);
		}
		
		private function getjump(at:int):int
		{
			var offset:int = Lua.ARGsBx(this._f.code[at]);
			if (offset == NO_JUMP)  /* point to itself represents end of list */
				return NO_JUMP;  /* end of list */
			else
				return (at+1)+offset;  /* turn offset into absolute position */
		}

		/** Equivalent to <code>luaK_jump</code>. */
		public function kJump():int
		{
			var old_jpc:int = this._jpc;  /* save list of jumps to here */
			this._jpc = NO_JUMP;
			var j:int = kCodeAsBx(Lua.OP_JMP, 0, NO_JUMP);
			j = kConcat(j, old_jpc);  /* keep them on hold */
			return j;
		}

		/** Equivalent to <code>luaK_storevar</code>. */
		public function kStorevar(_var:Expdesc, ex:Expdesc):void
		{
			switch (_var.k)
			{
				case Expdesc.VLOCAL:
					{
						freeexp(ex);
						exp2reg(ex, _var.info);
						return;
					}
			  
				case Expdesc.VUPVAL:
					{
						var e:int = kExp2anyreg(ex);
						kCodeABC(Lua.OP_SETUPVAL, e, _var.info, 0);
						break;
					}
			  
				case Expdesc.VGLOBAL:
					{
						var e2:int = kExp2anyreg(ex);
						kCodeABx(Lua.OP_SETGLOBAL, e2, _var.info);
						break;
					}
			  
				case Expdesc.VINDEXED:
					{
						var e3:int = kExp2RK(ex);
						kCodeABC(Lua.OP_SETTABLE, _var.info, _var.aux, e3);
						break;
					}
			  
				default:
					{
						/* invalid var kind to store */
						//# assert false
						break;
					}
			}
			freeexp(ex);
		}

		/** Equivalent to <code>luaK_indexed</code>. */
		public function kIndexed(t:Expdesc, k:Expdesc):void
		{
			t.aux = kExp2RK(k);
			t.k = Expdesc.VINDEXED;
		}
		
		/** Equivalent to <code>luaK_exp2RK</code>. */
		public function kExp2RK(e:Expdesc):int
		{
			kExp2val(e);
			switch (e.k)
			{
				case Expdesc.VKNUM:
				case Expdesc.VTRUE:
				case Expdesc.VFALSE:
				case Expdesc.VNIL:
					if (this._nk <= Lua.MAXINDEXRK)    /* constant fit in RK operand? */
					{
						e.info = (e.k == Expdesc.VNIL)  ? nilK() :
							(e.k == Expdesc.VKNUM) ? kNumberK(e.nval) :
							boolK(e.k == Expdesc.VTRUE);
						e.k = Expdesc.VK;
						return e.info | Lua.BITRK;
					}
					else 
						break;

				case Expdesc.VK:
					if (e.info <= Lua.MAXINDEXRK)  /* constant fit in argC? */
						return e.info | Lua.BITRK;
					else 
						break;

				default: 
					break;
			}
			/* not a constant in the right range: put it in a register */
			return kExp2anyreg(e);
		}
		
		/** Equivalent to <code>luaK_exp2val</code>. */
		public function kExp2val(e:Expdesc):void
		{
			if (e.hasjumps())
				kExp2anyreg(e);
			else
				kDischargevars(e);
		}

		private function boolK(b:Boolean):int
		{
			return addk(Lua.valueOfBoolean(b));
		}

		private function nilK():int
		{
			return addk(Lua.NIL);
		}
		
		/** Equivalent to <code>luaK_goiffalse</code>. */
		public function kGoiffalse(e:Expdesc):void
		{
			var lj:int;  /* pc of last jump */
			kDischargevars(e);
			switch (e.k)
			{
				case Expdesc.VNIL:
				case Expdesc.VFALSE:
					lj = NO_JUMP;  /* always false; do nothing */
					break;

				case Expdesc.VTRUE:
					lj = kJump();  /* always jump */
					break;

				case Expdesc.VJMP:
					lj = e.info;
					break;

				default:
					lj = jumponcond(e, true);
					break;
			}
			e.t = kConcat(e.t, lj);  /* insert last jump in `t' list */
			kPatchtohere(e.f);
			e.f = NO_JUMP;
		}
		
		/** Equivalent to <code>luaK_goiftrue</code>. */
		public function kGoiftrue(e:Expdesc):void
		{
			var lj:int;  /* pc of last jump */
			kDischargevars(e);
			switch (e.k)
			{
				case Expdesc.VK:
				case Expdesc.VKNUM:
				case Expdesc.VTRUE:
					lj = NO_JUMP;  /* always true; do nothing */
					break;

				case Expdesc.VFALSE:
					lj = kJump();  /* always jump */
					break;

				case Expdesc.VJMP:
					invertjump(e);
					lj = e.info;
					break;
				
				default:
					lj = jumponcond(e, false);
					break;
			}
			e.f = kConcat(e.f, lj);  /* insert last jump in `f' list */
			kPatchtohere(e.t);
			e.t = NO_JUMP;
		}

		private function invertjump(e:Expdesc):void
		{
			var at:int = getjumpcontrol(e.info);
			var code:Array = this._f.code; //int []
			var instr:int = code[at] ;
			//# assert testTMode(Lua.OPCODE(instr)) && Lua.OPCODE(instr) != Lua.OP_TESTSET && Lua.OPCODE(instr) != Lua.OP_TEST
			code[at] = Lua.SETARG_A(instr, (Lua.ARGA(instr) == 0 ? 1 : 0));
		}
		
		private function jumponcond(e:Expdesc, cond:Boolean):int
		{
			if (e.k == Expdesc.VRELOCABLE)
			{
				var ie:int = getcode(e);
				if (Lua.OPCODE(ie) == Lua.OP_NOT)
				{
					this._pc--;  /* remove previous OP_NOT */
					return condjump(Lua.OP_TEST, Lua.ARGB(ie), 0, cond ? 0 : 1);
			  }
			  /* else go through */
			}
			discharge2anyreg(e);
			freeexp(e);
			return condjump(Lua.OP_TESTSET, Lua.NO_REG, e.info, cond ? 1 : 0);
		}

		private function condjump(op:int, a:int, b:int, c:int):int
		{
			kCodeABC(op, a, b, c);
			return kJump();
		}

		private function discharge2anyreg(e:Expdesc):void
		{
			if (e.k != Expdesc.VNONRELOC)
			{
				kReserveregs(1);
				discharge2reg(e, this._freereg - 1);
			}
		}


		public function kSelf(e:Expdesc, key:Expdesc):void
		{
			kExp2anyreg(e);
			freeexp(e);
			var func:int = this._freereg;
			kReserveregs(2);
			kCodeABC(Lua.OP_SELF, func, e.info, kExp2RK(key));
			freeexp(key);
			e.info = func;
			e.k = Expdesc.VNONRELOC;
		}

		public function kSetlist(base:int, nelems:int, tostore:int):void
		{
			var c:int =  (nelems - 1) / Lua.LFIELDS_PER_FLUSH + 1;
			var b:int = (tostore == Lua.MULTRET) ? 0 : tostore;
			//# assert tostore != 0
			if (c <= Lua.MAXARG_C)
				kCodeABC(Lua.OP_SETLIST, base, b, c);
			else
			{
				kCodeABC(Lua.OP_SETLIST, base, b, 0);
				kCode(c, this._ls.lastline);
			}
			this._freereg = base + 1;  /* free registers with list values */
		}


		public function codecomp(op:int, cond:Boolean, e1:Expdesc, e2:Expdesc):void
		{
			var o1:int = kExp2RK(e1);
			var o2:int = kExp2RK(e2);
			freeexp(e2);
			freeexp(e1);
			if ((!cond) && op != Lua.OP_EQ)
			{
				/* exchange args to replace by `<' or `<=' */
				var temp:int = o1; 
				o1 = o2; 
				o2 = temp;  /* o1 <==> o2 */
				cond = true;
			}
			e1.info = condjump(op, (cond ? 1 : 0), o1, o2);
			e1.k = Expdesc.VJMP;
		}

		public function markupval(level:int):void
		{
			var b:BlockCnt = this.bl;
			while (b != null && b.nactvar > level)
				b = b.previous;
			if (b != null)
				b.upval = true;
		}
		
		//新增
		public function get f():Proto
		{
			return this._f;
		}
		
		//新增
		public function set f(f:Proto):void
		{
			this._f = f;
		}
		
		//新增
		public function get prev():FuncState
		{
			return this._prev;
		}
		
		//新增
		public function set prev(prev:FuncState):void
		{
			this._prev = prev;
		}

		//新增
		public function set ls(ls:Syntax):void
		{
			this._ls = ls;
		}
		
		//新增
		public function set L(L:Lua):void
		{
			this._L = L;
		}
		
		//新增
		public function get bl():BlockCnt
		{
			return this._bl;
		}
		
		//新增
		public function set bl(bl:BlockCnt):void
		{
			this._bl = bl;
		}
		
		//新增
		public function get pc():int
		{
			return this._pc;
		}		
		
		//新增
		public function get np():int
		{
			return this._np;
		}			
		//新增
		public function set np(np:int):void
		{
			this._np = np;
		}
		
		//新增
		public function get nlocvars():int
		{
			return this._nlocvars;
		}			
		//新增
		public function set nlocvars(nlocvars:int):void
		{
			this._nlocvars = nlocvars;
		}	
		
		
		//新增
		public function get nactvar():int
		{
			return this._nactvar;
		}			
		//新增
		public function set nactvar(nactvar:int):void
		{
			this._nactvar = nactvar;
		}
		
		//新增
		public function get upvalues():Array
		{
			return this._upvalues;
		}		
		
		//新增
		public function get actvar():Array
		{
			return this._actvar;
		}				
	}
}