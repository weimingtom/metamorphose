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
	import com.iteye.weimingtom.metamorphose.java.Character;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	
	public final class MatchState
	{
		private var _L:Lua;
		/** The entire string that is the subject of the match. */
		private var _src:String;
		/** The subject's length. */
		private var _end:int;
		/** Total number of captures (finished or unfinished). */
		private var _level:int;
		/** Each capture element is a 2-element array of (index, len). */
		private var _capture:Array = new Array();
		// :todo: consider adding the pattern string as a member (and removing
		// p parameter from methods).

		// :todo: consider removing end parameter, if end always == // src.length()
		public function MatchState(L:Lua, src:String, end:int)
		{
			this._L = L;
			this._src = src;
			this._end = end;
		}

		/**
		 * Returns the length of capture <var>i</var>.
		 */
		private function captureLen(i:int):int
		{
			var c:Array = this._capture.elementAt(i) as Array; //(int[])
			return c[1];
		}
		
		/**
		 * Returns the init index of capture <var>i</var>.
		 */
		private function captureInit(i:int):int
		{
			var c:Array = this._capture.elementAt(i) as Array; //(int[])
			return c[0];
		}
		
		/**
		 * Returns the 2-element array for the capture <var>i</var>.
		 */
		private function capture(i:int):Array //int[]
		{
			return this._capture.elementAt(i) as Array;//(int[])
		}

		public function capInvalid():int
		{
			return this._L.error("invalid capture index");
		}
		
		public function malBra():int
		{
			return this._L.error("malformed pattern (missing '[')");
		}

		public function capUnfinished():int
		{
			return this._L.error("unfinished capture");
		}

		public function malEsc():int 
		{
			return this._L.error("malformed pattern (ends with '%')");
		}

		public function check_capture(l:uint):uint
		{
			l -= '1'.charCodeAt();   // relies on wraparound.
			if (l >= this._level || captureLen(l) == CAP_UNFINISHED)
				capInvalid();
			return l;
		}

		public function capture_to_close():int
		{
			var lev:int = this._level;
			for (lev--; lev>=0; lev--)
				if (captureLen(lev) == CAP_UNFINISHED)
					return lev;
			return capInvalid();
		}

		public function classend(p:String, pi:int):int 
		{
			switch (p.charAt(pi++))
			{
				case String.fromCharCode(L_ESC):
					// assert pi < p.length() // checked by callers
					return pi+1;

				case '[':
					if (p.length == pi)
						return malBra();
					if (p.charAt(pi) == '^')
						++pi;
					do    // look for a ']'
					{
						if (p.length == pi)
							return malBra();
						if (p.charCodeAt(pi++) == L_ESC)
						{
							if (p.length == pi)
								return malBra();
							++pi;     // skip escapes (e.g. '%]')
							if (p.length == pi)
								return malBra();
						}
					} while (p.charAt(pi) != ']');
					return pi+1;
				
				default:
					return pi;
			}
		}

		/**
		 * @param c   char match.
		 * @param cl  character class.
		 */
		public static function match_class(c:uint, cl:uint):Boolean
		{
			var res:Boolean;
			switch (Character.toLowerCase(cl))
			{
				case 'a' : 
					res = Syntax.isalpha(c); 
					break;
		  
				case 'c' : 
					res = Syntax.iscntrl(c); 
					break;
		  
				case 'd' : 
					res = Syntax.isdigit(c); 
					break;
		  
				case 'l' : 
					res = Syntax.islower(c); 
					break;
		  
				case 'p' : 
					res = Syntax.ispunct(c); 
					break;
		  
				case 's' : 
					res = Syntax.isspace(c); 
					break;
					
				case 'u' : 
					res = Syntax.isupper(c); 
					break;
		  
				case 'w' : 
					res = Syntax.isalnum(c); 
					break;
		  
				case 'x' : 
					res = Syntax.isxdigit(c); 
					break;
		  
				case 'z' : 
					res = (c == 0); 
					break;
				
				default: 
					return (cl == c);
			}
			return Character.isLowerCase(cl) ? res : !res;
		}
		
		/**
		 * @param pi  index in p of start of class.
		 * @param ec  index in p of end of class.
		 */
		public static function matchbracketclass(c:uint, p:String, pi:int, ec:int):Boolean
		{
			// :todo: consider changing char c to int c, then -1 could be used
			// represent a guard value at the beginning and end of all strings (a
			// better NUL).  -1 of course would match no positive class.

			// assert p.charAt(pi) == '[';
			// assert p.charAt(ec) == ']';
			var sig:Boolean = true;
			if (p.charCodeAt(pi+1) == '^'.charCodeAt())
			{
				sig = false;
				++pi;     // skip the '6'
			}
			while (++pi < ec)
			{
				if (p.charCodeAt(pi) == L_ESC)
				{
					++pi;
					if (match_class(c, p.charCodeAt(pi)))
						return sig;
				}
				else if ((p.charAt(pi+1) == '-') && (pi+2 < ec))
				{
					pi += 2;
					if (p.charCodeAt(pi-2) <= c && c <= p.charCodeAt(pi))
						return sig;
				}
				else if (p.charCodeAt(pi) == c)
				{
					return sig;
				}
			}
			return !sig;
		}
		
		public static function singlematch(c:uint, p:String, pi:int, ep:int):Boolean
		{
			switch (p.charAt(pi))
			{
				case '.': 
					return true;    // matches any char
		  
				case String.fromCharCode(L_ESC): 
					return match_class(c, p.charCodeAt(pi+1));
		  
				case '[': 
					return matchbracketclass(c, p, pi, ep-1);
		  
				default: 
					return p.charCodeAt(pi) == c;
			}
		}

		// Generally all the various match functions from PUC-Rio which take a
		// MatchState and return a "const char *" are transformed into
		// instance methods that take and return string indexes.
		
		public function matchbalance(si:int, p:String, pi:int):int
		{
			if (pi + 1 >= p.length)
				this._L.error("unbalanced pattern");
			if (si >= this._end || this._src.charAt(si) != p.charAt(pi))
			{
				return -1;
			}
			var b:uint = p.charCodeAt(pi);
			var e:uint = p.charCodeAt(pi+1);
			var cont:int = 1;
			while (++si < this._end)
			{
				if (this._src.charCodeAt(si) == e)
				{
					if (--cont == 0)
						return si+1;
				}
				else if (this._src.charCodeAt(si) == b)
				{
					++cont;
				}
			}
			return -1;  // string ends out of balance
		}
		
		public function max_expand(si:int, p:String, pi:int, ep:int):int
		{
			var i:int = 0;  // counts maximum expand for item
			while (si + i < this._end && singlematch(this._src.charCodeAt(si+i), p, pi, ep))
			{
				++i;
			}
			// keeps trying to match with the maximum repetitions
			while (i >= 0)
			{
				var res:int = match(si+i, p, ep+1);
				if (res >= 0)
					return res;
				--i;      // else didn't match; reduce 1 repetition to try again
			}
			return -1;
		}

		public function min_expand(si:int, p:String, pi:int, ep:int):int
		{
			while (true)
			{
				var res:int = match(si, p, ep+1);
				if (res >= 0)
					return res;
				else if (si < this._end && singlematch(this._src.charCodeAt(si), p, pi, ep))
					++si;   // try with one more repetition
				else
					return -1;
			}
			
			//unreachable
			return -1;
		}
		
		public function start_capture(si:int, p:String, pi:int, what:int):int
		{
			_capture.setSize(this._level + 1);
			_capture.setElementAt([si, what], this._level);
			++this._level;
			var res:int = match(si, p, pi);
			if (res < 0)        // match failed
			{
				--this._level;
			}
			return res;
		}

		public function end_capture(si:int, p:String, pi:int):int
		{
			var l:int = capture_to_close();
			capture(l)[1] = si - captureInit(l);        // close it
			var res:int = match(si, p, pi);
			if (res < 0)        // match failed?
			{
				capture(l)[1] = CAP_UNFINISHED;   // undo capture
			}
			return res;
		}

		public function match_capture(si:int, l:uint):int
		{
			l = check_capture(l);
			var len:int = captureLen(l);
			if (this._end - si >= len) //TODO: 
/*              &&
				src.regionMatches(false,
					captureInit(l),
					src,
					si,
					len))*/
			{
				return si+len;
			}
			return -1;
		}

		public static const L_ESC:uint = '%'.charCodeAt();
		public static const SPECIALS:String = "^$*+?.([%-";
		private static const CAP_UNFINISHED:int = -1;
		private static const CAP_POSITION:int = -2;
		
		/**
		 * @param si  index of subject at which to attempt match.
		 * @param p   pattern string.
		 * @param pi  index into pattern (from which to being matching).
		 * @return the index of the end of the match, -1 for no match.
		 */
		public function match(si:int, p:String, pi:int):int
		{
			// This code has been considerably changed in the transformation
			// from C to Java.  There are the following non-obvious changes:
			// - The C code routinely relies on NUL being accessible at the end of
			//   the pattern string.  In Java we can't do this, so we use many
			//   more explicit length checks and pull error cases into this
			//   function.  :todo: consider appending NUL to the pattern string.
			// - The C code uses a "goto dflt" which is difficult to transform in
			//   the usual way.
init:   // labelled while loop emulates "goto init", which we use to
		// optimize tail recursion.
			while (true)
			{
				if (p.length == pi)     // end of pattern
				return si;              // match succeeded
				switch (p.charAt(pi))
				{
					case '(':
						if (p.length == pi + 1)
						{
							return capUnfinished();
						}
						if (p.charAt(pi+1) == ')')  // position capture?
							return start_capture(si, p, pi+2, CAP_POSITION);
						return start_capture(si, p, pi+1, CAP_UNFINISHED);

					case ')':       // end capture
						return end_capture(si, p, pi+1);

					case String.fromCharCode(L_ESC):
						if (p.length == pi + 1)
						{
							return malEsc();
						}
						switch (p.charAt(pi+1))
						{
							case 'b':   // balanced string?
								si = matchbalance(si, p, pi+2);
								if (si < 0)
									return si;
								pi += 4;
								// else return match(ms, s, p+4);
								continue init;    // goto init
							
							case 'f':   // frontier
								{
									pi += 2;
									if (p.length == pi || p.charAt(pi) != '[')
										return this._L.error("missing '[' after '%f' in pattern");
									var ep:int = classend(p, pi);   // indexes what is next
									var previous:uint  = (si == 0) ? '\0'.charCodeAt() : this._src.charCodeAt(si-1);
									var at:uint = (si == this._end) ? '\0'.charCodeAt() : this._src.charCodeAt(si);
									if (matchbracketclass(previous, p, pi, ep-1) ||
										!matchbracketclass(at, p, pi, ep-1))
									{
										return -1;
									}
									pi = ep;
									// else return match(ms, s, ep);
								}
								continue init;    // goto init
							
							default:
								if (Syntax.isdigit(p.charCodeAt(pi+1))) // capture results (%0-%09)?
								{
									si = match_capture(si, p.charCodeAt(pi+1));
									if (si < 0)
										return si;
									pi += 2;
									// else return match(ms, s, p+2);
									continue init;  // goto init
								}
							// We emulate a goto dflt by a fallthrough to the next
							// case (of the outer switch) and making sure that the
							// next case has no effect when we fallthrough to it from here.
							// goto dflt;
						}
				
					// FALLTHROUGH
					case '$':
						if (p.charAt(pi) == '$')
						{
							if (p.length == pi+1)      // is the '$' the last char in pattern?
								return (si == this._end) ? si : -1;     // check end of string
							// else goto dflt;
						}
						
					// FALLTHROUGH
					default:        // it is a pattern item
						{
							var ep2:int = classend(p, pi);   // indexes what is next
							var m:Boolean = si < this._end && singlematch(this._src.charCodeAt(si), p, pi, ep);
							if (p.length > ep2)
							{
								switch (p.charAt(ep2))
								{
									case '?':       // optional
										if (m)
										{
											var res:int = match(si+1, p, ep2+1);
											if (res >= 0)
												return res;
										}
										pi = ep2 + 1;
										// else return match(s, ep+1);
										continue init;      // goto init

									case '*':       // 0 or more repetitions
										return max_expand(si, p, pi, ep2);

									case '+':       // 1 or more repetitions
										return m ? max_expand(si+1, p, pi, ep2) : -1;

									case '-':       // 0 or more repetitions (minimum)
										return min_expand(si, p, pi, ep2);
								}
							}
							// else or default:
							if (!m)
								return -1;
							++si;
							pi = ep2;
							// return match(ms, s+1, ep);
							continue init;
						}
				}
			}
			
			//unreachable
			return -1;
		}

		/**
		 * @param s  index of start of match.
		 * @param e  index of end of match.
		 */
		public function onecapture(i:int, s:int, e:int):Object
		{
			if (i >= this._level)
			{
				if (i == 0)       // level == 0, too
					return this._src.substring(s, e);    // add whole match
				else
					capInvalid();
					// NOTREACHED;
			}
			var l:int = captureLen(i);
			if (l == CAP_UNFINISHED)
				capUnfinished();
			if (l == CAP_POSITION)
				return Lua.valueOfNumber(captureInit(i) +1);
			return this._src.substring(captureInit(i), captureInit(i) + l);
		}

		public function push_onecapture(i:int, s:int, e:int):void 
		{
			this._L.pushObject(onecapture(i, s, e));
		}

		/**
		 * @param s  index of start of match.
		 * @param e  index of end of match.
		 */
		public function push_captures(s:int, e:int):int 
		{
			var nlevels:int = (this._level == 0 && s >= 0) ? 1 : this._level;
			for (var i:int = 0; i < nlevels; ++i)
				push_onecapture(i, s, e);
			return nlevels;     // number of strings pushed
		}

		/** A helper for gsub.  Equivalent to add_s from lstrlib.c. */
		public function adds(b:StringBuffer, si:int, ei:int):void
		{
			var news:String = this._L.toString(this._L.value(3));
			var l:int = news.length;
			for (var i:int = 0; i < l; ++i)
			{
				if (news.charCodeAt(i) != L_ESC)
				{
					b.append(news.charCodeAt(i));
				}
				else
				{
					++i;    // skip L_ESC
					if (!Syntax.isdigit(news.charCodeAt(i)))
					{
						b.append(news.charCodeAt(i));
					}
					else if (news.charAt(i) == '0')
					{
						b.appendString(this._src.substring(si, ei));
					}
					else
					{
						// add capture to accumulated result
						b.appendString(this._L.toString(onecapture(news.charCodeAt(i) - '1'.charCodeAt(), si, ei)));
					}
				}
			}
		}

		/** A helper for gsub.  Equivalent to add_value from lstrlib.c. */
		public function addvalue(b:StringBuffer, si:int, ei:int):void 
		{
			switch (this._L.type(3))
			{
				case Lua.TNUMBER:
				case Lua.TSTRING:
					adds(b, si, ei);
					return;
				
				case Lua.TFUNCTION:
					{
						this._L.pushValue(3);
						var n:int = push_captures(si, ei);
						this._L.call(n, 1);
					}
					break;
					
				case Lua.TTABLE:
					this._L.pushObject(this._L.getTable(this._L.value(3), onecapture(0, si, ei)));
					break;
					
				default:
					{
						this._L.argError(3, "string/function/table expected");
						return;
					}
			}
			if (!this._L.toBoolean(this._L.value(-1)))      // nil or false
			{
				this._L.pop(1);
				this._L.pushString(this._src.substring(si, ei));
			}
			else if (!Lua.isString(this._L.value(-1)))
			{
				this._L.error("invalid replacement value (a " +
					Lua.typeName(this._L.type(-1)) + ")");
			}
			b.appendString(this._L.toString(this._L.value(-1)));  // add result to accumulator
			this._L.pop(1);
		}
		
		//新增
		public function get end():int
		{
			return this._end;
		}
		
		//新增
		public function set level(level:int):void
		{
			this._level = level;
		}
		

	}
}