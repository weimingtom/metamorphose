/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/Syntax.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.Hashtable;
	import com.iteye.weimingtom.metamorphose.java.NumberFormatException;
	import com.iteye.weimingtom.metamorphose.java.Reader;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	
	/**
	 * Syntax analyser.  Lexing, parsing, code generation.
	 */
	public final class Syntax
	{
		/** End of File, must be -1 as that is what read() returns. */
		private static const EOZ:int = -1;

		private static const FIRST_RESERVED:int = 257;

		// WARNING: if you change the order of this enumeration,
		// grep "ORDER RESERVED"
		private static const TK_AND:int       = FIRST_RESERVED + 0;
		private static const TK_BREAK:int     = FIRST_RESERVED + 1;
		private static const TK_DO:int        = FIRST_RESERVED + 2;
		private static const TK_ELSE:int      = FIRST_RESERVED + 3;
		private static const TK_ELSEIF:int    = FIRST_RESERVED + 4;
		private static const TK_END:int       = FIRST_RESERVED + 5;
		private static const TK_FALSE:int     = FIRST_RESERVED + 6;
		private static const TK_FOR:int       = FIRST_RESERVED + 7;
		private static const TK_FUNCTION:int  = FIRST_RESERVED + 8;
		private static const TK_IF:int        = FIRST_RESERVED + 9;
		private static const TK_IN:int        = FIRST_RESERVED + 10;
		private static const TK_LOCAL:int     = FIRST_RESERVED + 11;
		private static const TK_NIL:int       = FIRST_RESERVED + 12;
		private static const TK_NOT:int       = FIRST_RESERVED + 13;
		private static const TK_OR:int        = FIRST_RESERVED + 14;
		private static const TK_REPEAT:int    = FIRST_RESERVED + 15;
		private static const TK_RETURN:int    = FIRST_RESERVED + 16;
		private static const TK_THEN:int      = FIRST_RESERVED + 17;
		private static const TK_TRUE:int      = FIRST_RESERVED + 18;
		private static const TK_UNTIL:int     = FIRST_RESERVED + 19;
		private static const TK_WHILE:int     = FIRST_RESERVED + 20;
		private static const TK_CONCAT:int    = FIRST_RESERVED + 21;
		private static const TK_DOTS:int      = FIRST_RESERVED + 22;
		private static const TK_EQ:int        = FIRST_RESERVED + 23;
		private static const TK_GE:int        = FIRST_RESERVED + 24;
		private static const TK_LE:int        = FIRST_RESERVED + 25;
		private static const TK_NE:int        = FIRST_RESERVED + 26;
		private static const TK_NUMBER:int    = FIRST_RESERVED + 27;
		private static const TK_NAME:int      = FIRST_RESERVED + 28;
		private static const TK_STRING:int    = FIRST_RESERVED + 29;
		private static const TK_EOS:int       = FIRST_RESERVED + 30;

		private static const NUM_RESERVED:int = TK_WHILE - FIRST_RESERVED + 1;

		/** Equivalent to luaX_tokens.  ORDER RESERVED */
		private static var _tokens:Array = //new String[]
		[
			"and", "break", "do", "else", "elseif",
			"end", "false", "for", "function", "if",
			"in", "local", "nil", "not", "or", "repeat",
			"return", "then", "true", "until", "while",
			"..", "...", "==", ">=", "<=", "~=",
			"<number>", "<name>", "<string>", "<eof>"
		];

		private static var _reserved:Hashtable;
		
		//TODO:实现静态初始化
		public static function init():void
		{
			if(Syntax._reserved == null)
			{
				Syntax._reserved = new Hashtable();
				
				for (var i:int = 0; i < NUM_RESERVED; ++i)
				{
					//TODO:
					Syntax._reserved.put(Syntax._tokens[i], new int(FIRST_RESERVED+i));
				}
			}
		}

		// From struct LexState

		/** current character */
		private var _current:int;
		/** input line counter */
		private var _linenumber:int = 1;
		/** line of last token 'consumed' */
		private var _lastline:int = 1;
		/**
		* The token value.  For "punctuation" tokens this is the ASCII value
		* for the character for the token; for other tokens a member of the
		* enum (all of which are > 255).
		*/
		private var _token:int = 0;
		/** Semantic info for token; a number. */
		private var _tokenR:Number = 0;
		/** Semantic info for token; a string. */
		private var _tokenS:String;

		/** Lookahead token value. */
		private var _lookahead:int = TK_EOS;
		/** Semantic info for lookahead; a number. */
		private var _lookaheadR:Number = 0;
		/** Semantic info for lookahead; a string. */
		private var _lookaheadS:String;

		/** Semantic info for return value from {@link #llex}; a number. */
		private var _semR:Number = 0;
		/** As {@link #semR}, for string. */
		private var _semS:String;

		/** FuncState for current (innermost) function being parsed. */
		private var _fs:FuncState;
		private var _L:Lua;

		/** input stream */
		private var _z:Reader;

		/** Buffer for tokens. */
		private var _buff:StringBuffer = new StringBuffer();

		/** current source name */
		public var _source:String;

		/** locale decimal point. */
		//TODO:这个变量貌似没有使用
		private var _decpoint:uint = '.'.charCodeAt();
		
		public function Syntax(L:Lua, z:Reader, source:String) 
		{
			Syntax.init();
			this._L = L;
			this._z = z;
			this._source = source;
			next();	
		}
		
		public function get lastline():int
		{
			return this._lastline;
		}
		
		// From <ctype.h>

		// Implementations of functions from <ctype.h> are only correct copies
		// to the extent that Lua requires them.
		// Generally they have default access so that StringLib can see them.
		// Unlike C's these version are not locale dependent, they use the
		// ISO-Latin-1 definitions from CLDC 1.1 Character class.

		public static function isalnum(c:int):Boolean
		{
			var ch:uint = c as uint;
			return Character.isUpperCase(ch) ||
				Character.isLowerCase(ch) ||
				Character.isDigit(ch);
		}

		public static function isalpha(c:int):Boolean
		{
			var ch:uint = c as uint;
			return Character.isUpperCase(ch) ||
				Character.isLowerCase(ch);
		}

		/** True if and only if the char (when converted from the int) is a
		 * control character.
		 */
		public static function iscntrl(c:int):Boolean
		{
			return (c as uint) < 0x20 || (c as uint) == 0x7f;
		}

		public static function isdigit(c:int):Boolean
		{
			return Character.isDigit(c as uint);
		}

		public static function islower(c:int):Boolean
		{
			return Character.isLowerCase(c as uint);
		}

		/**
		 * A character is punctuation if not cntrl, not alnum, and not space.
		 */
		public static function ispunct(c:int):Boolean
		{
			return !isalnum(c) && !iscntrl(c) && !isspace(c);
		}

		public static function isspace(c:int):Boolean
		{
			return c == ' '.charCodeAt() ||
				   c == '\f'.charCodeAt() ||
				   c == '\n'.charCodeAt() ||
				   c == '\r'.charCodeAt() ||
				   c == '\t'.charCodeAt();
		}

		public static function isupper(c:int):Boolean
		{
			return Character.isUpperCase(c as uint);
		}

		public static function isxdigit(c:int):Boolean 
		{
			return Character.isDigit(c as uint) ||
				('a'.charCodeAt() <= c && c <= 'f'.charCodeAt()) ||
				('A'.charCodeAt() <= c && c <= 'F'.charCodeAt());
		}

		// From llex.c
		
		private function check_next(_set:String):Boolean // throws IOException
		{
			if (_set.indexOf(String.fromCharCode(this._current)) < 0)
			{
				return false;
			}
			save_and_next();
			return true;
		}

		private function currIsNewline():Boolean 
		{
			return this._current == '\n'.charCodeAt() || 
				this._current == '\r'.charCodeAt();
		}

		private function inclinenumber():void // throws IOException
		{
			var old:int = this._current;
			//# assert currIsNewline()
			next();     // skip '\n' or '\r'
			if (currIsNewline() && this._current != old)
			{
				next();   // skip '\n\r' or '\r\n'
			}
			if (++this._linenumber < 0)       // overflow
			{
				xSyntaxerror("chunk has too many lines");
			}
		}

		private function skip_sep():int // throws IOException
		{
			var count:int = 0;
			var s:int = this._current;
			//# assert s == '[' || s == ']'
			save_and_next();
			while (this._current == '='.charCodeAt())
			{
				save_and_next();
				count++;
			}
			return (this._current == s) ? count : (-count) - 1;
		}

		private function read_long_string(isString:Boolean, sep:int):void // throws IOException
		{
			var cont:int = 0;
			save_and_next();  /* skip 2nd `[' */
			if (currIsNewline())  /* string starts with a newline? */
				inclinenumber();  /* skip it */
		loop:
			while (true)
			{
				switch (String.fromCharCode(this._current))
				{
					case String.fromCharCode(EOZ): //TODO:
						xLexerror(isString ? "unfinished long string" :
							"unfinished long comment",
							TK_EOS);
						break;  /* to avoid warnings */
			
					case ']':
						if (skip_sep() == sep)
						{
							save_and_next();  /* skip 2nd `]' */
							break loop;
						}
						break;
						
					case '\n':
					case '\r':
						__save('\n'.charCodeAt());
						inclinenumber();
						if (!isString)
							this._buff.setLength(0) ; /* avoid wasting space */
						break;

					default:
						if (isString) 
							save_and_next();
						else 
							next();
				}
			} /* loop */
			if (isString)
			{
				var rawtoken:String = this._buff.toString();
				var trim_by:int = 2 + sep ;
				this._semS = rawtoken.substring(trim_by, rawtoken.length - trim_by);
			}
		}

		/** Lex a token and return it.  The semantic info for the token is
		 * stored in <code>this.semR</code> or <code>this.semS</code> as
		 * appropriate.
		 */
		private function llex():int // throws IOException
		{
			if (Lua.D)
			{
				trace("llex() enter, current:" + this._current);
			}
			this._buff.setLength(0);
			while (true)
			{
				switch (String.fromCharCode(this._current))
				{
					case '\n':
					case '\r':
						if (Lua.D)
						{
							trace("case \\n\\r");
						}
						inclinenumber();
						continue;
				
					case '-':
						if (Lua.D)
						{
							trace("case -");
						}
						next();
						if (this._current != '-'.charCodeAt())
							return '-'.charCodeAt();
						/* else is a comment */
						next();
						if (this._current == '['.charCodeAt())
						{
							var sep2:int = skip_sep();
							this._buff.setLength(0) ; /* `skip_sep' may dirty the buffer */
							if (sep2 >= 0)
							{
								read_long_string(false, sep2);  /* long comment */
								this._buff.setLength(0) ;
								continue;
							}
						}
						/* else short comment */
						while (!currIsNewline() && this._current != EOZ)
							next();
						continue;

					case '[':
						if (Lua.D)
						{
							trace("case [");
						}
						var sep:int = skip_sep();
						if (sep >= 0)
						{
							read_long_string(true, sep);
							return TK_STRING;
						}
						else if (sep == -1)
							return '['.charCodeAt();
						else
							xLexerror("invalid long string delimiter", TK_STRING);
						continue;     // avoids Checkstyle warning.
				
					case '=':
						if (Lua.D)
						{
							trace("case =");
						}
						next() ;
						if (this._current != '='.charCodeAt())
						{ 
							return '='.charCodeAt(); 
						}
						else
						{
							next() ;
							return TK_EQ ;
						}
					
					case '<':
						if (Lua.D)
						{
							trace("case <");
						}
						next();
						if (this._current != '='.charCodeAt())
						{ 
							return '<'.charCodeAt(); 
						}
						else
						{
							next() ;
							return TK_LE ;
						}
					
					case '>':
						if (Lua.D)
						{
							trace("case >");
						}
						next() ;
						if (this._current != '='.charCodeAt())
						{ 
							return '>'.charCodeAt(); 
						}
						else
						{
							next() ;
							return TK_GE ;
						}
					
					case '~':
						if (Lua.D)
						{
							trace("case ~");
						}
						next();
						if (this._current != '='.charCodeAt())
						{ 
							return '~'.charCodeAt(); 
						}
						else
						{
							next();
							return TK_NE;
						}
					
					case '"':
					case '\'':
						if (Lua.D)
						{
							trace("case \"'");
						}
						read_string(this._current);
						return TK_STRING;
				
					case '.':
						if (Lua.D)
						{
							trace("case .");
						}
						save_and_next();
						if (check_next("."))
						{
							if (check_next("."))
							{
								return TK_DOTS;
							}
							else
							{
								return TK_CONCAT ;
							}
						}
						else if (!isdigit(this._current))
						{
							return '.'.charCodeAt();
						}
						else
						{
							read_numeral();
							return TK_NUMBER;
						}
					
					case String.fromCharCode(EOZ): //TODO:
						if (Lua.D)
						{
							trace("case EOZ");
						}
						return TK_EOS;
					
					default:
						if (isspace(this._current))
						{
							if (Lua.D)
							{
								trace("isspace");
							}
							// assert !currIsNewline();
							next();
							continue;
						}
						else if (isdigit(this._current))
						{
							if (Lua.D)
							{
								trace("isdigit");
							}
							read_numeral();
							return TK_NUMBER;
						}
						else if (isalpha(this._current) || this._current == '_'.charCodeAt())
						{
							if (Lua.D)
							{
								trace("isalpha or _");
							}
							// identifier or reserved word
							do
							{
								save_and_next();
							} while (isalnum(this._current) || this._current == '_'.charCodeAt());
							var s:String = this._buff.toString();
							var t:Object = Syntax._reserved._get(s); //TODO:
							if (t == null)
							{
								this._semS = s;
								return TK_NAME;
							}
							else
							{
								return t as int;
							}
						}
						else
						{
							var c:int = this._current;
							next();
							return c; // single-char tokens
						}
				}
			}
			//unreachable
			return 0;
		}

		private function next():void //throws IOException
		{
			this._current = this._z.read();
			if (Lua.D) 
			{
				trace("Syntax.next(), current:" + this._current + "(" + String.fromCharCode(this._current) +")");
			}
		}

		/** Reads number.  Writes to semR. */
		private function read_numeral():void // throws IOException
		{
			// assert isdigit(current);
			do
			{
				save_and_next();
			} while (isdigit(this._current) || this._current == '.'.charCodeAt());
			if (check_next("Ee"))       // 'E' ?
			{
				check_next("+-"); // optional exponent sign
			}
			while (isalnum(this._current) || this._current == '_'.charCodeAt())
			{
				save_and_next();
			}
			// :todo: consider doing PUC-Rio's decimal point tricks.
			try
			{
				this._semR = Number(this._buff.toString());
				return;
			}
			catch (e:NumberFormatException)
			{
				trace(e.getStackTrace());
				xLexerror("malformed number", TK_NUMBER);
			}
		}

		/** Reads string.  Writes to semS. */
		private function read_string(del:int):void // throws IOException
		{
			save_and_next();
			while (this._current != del)
			{
				switch (String.fromCharCode(this._current))
				{
					case String.fromCharCode(EOZ): //TODO:
						xLexerror("unfinished string", TK_EOS);
						continue;     // avoid compiler warning
					 
					case '\n':
					case '\r':
						xLexerror("unfinished string", TK_STRING);
						continue;     // avoid compiler warning
					
					case '\\':
						{
							var c:int;
							next();       // do not save the '\'
							switch (String.fromCharCode(this._current))
							{
								case 'a': 
									c = 7; 
									break;     // no '\a' in Java.
						
								case 'b': 
									c = '\b'.charCodeAt(); 
									break;
						
								case 'f': 
									c = '\f'.charCodeAt(); 
									break;
						
								case 'n': 
									c = '\n'.charCodeAt(); 
									break;
						
								case 'r': 
									c = '\r'.charCodeAt(); 
									break;
						
								case 't': 
									c = '\t'.charCodeAt(); 
									break;
						
								case 'v': 
									c = 11; 
									break;    // no '\v' in Java.
						
								case '\n': case '\r':
									__save('\n'.charCodeAt());
									inclinenumber();
									continue;
						
								case String.fromCharCode(EOZ):
									continue; // will raise an error next loop
						
								default:
									if (!isdigit(this._current))
									{
										save_and_next();        // handles \\, \", \', \?
									}
									else    // \xxx
									{
										var i:int = 0;
										c = 0;
										do
										{
											c = 10*c + (this._current - '0'.charCodeAt());
											next();
										} while (++i<3 && isdigit(this._current));
										// In unicode, there are no bounds on a 3-digit decimal.
										__save(c);
									}
									continue;
							}
							__save(c);
							next();
							continue;
						}
					
					default:
						save_and_next();
				}
			}
			save_and_next();    // skip delimiter
			var rawtoken:String = this._buff.toString() ;
			this._semS = rawtoken.substring(1, rawtoken.length - 1) ;
		}

		private function save():void
		{
			this._buff.append(this._current as uint);
		}

		private function __save(c:int):void
		{
			this._buff.append(c as uint);
		}

		private function save_and_next():void  // throws IOException
		{
			save();
			next();
		}

		/** Getter for source. */
		public function get source():String
		{
			return _source;
		}

		private function txtToken(tok:int):String
		{
			switch (tok)
			{
				case TK_NAME:
				case TK_STRING:
				case TK_NUMBER:
					return this._buff.toString();
			  
				default:
					return xToken2str(tok);
			}
		}

		/** Equivalent to <code>luaX_lexerror</code>. */
		private function xLexerror(msg:String, tok:int):void
		{
			msg = source + ":" + this._linenumber + ": " + msg;
			if (tok != 0)
			{
				msg = msg + " near '" + txtToken(tok) + "'";
			}
			this._L.pushString(msg);
			this._L.dThrow(Lua.ERRSYNTAX);
		}

		/** Equivalent to <code>luaX_next</code>. */
		private function xNext():void // throws IOException
		{
			this._lastline = this._linenumber;
			if (this._lookahead != TK_EOS)      // is there a look-ahead token?
			{
				this._token = this._lookahead;        // Use this one,
				this._tokenR = this._lookaheadR;
				this._tokenS = this._lookaheadS;
				this._lookahead = TK_EOS;       // and discharge it.
			}
			else
			{
				this._token = llex();
				this._tokenR = this._semR;
				this._tokenS = this._semS;
			}
		}

		/** Equivalent to <code>luaX_syntaxerror</code>. */
		public function xSyntaxerror(msg:String):void
		{
			xLexerror(msg, this._token);
		}

		private static function xToken2str(token:int):String
		{
			if (token < FIRST_RESERVED)
			{
				// assert token == (char)token;
				if (iscntrl(token))
				{
					return "char(" + token + ")";
				}
				return String.fromCharCode(token as uint);
			}
			return Syntax._tokens[token - FIRST_RESERVED];
		}

		// From lparser.c

		private static function block_follow(token:int):Boolean
		{
			switch (token)
			{
				case TK_ELSE: case TK_ELSEIF: case TK_END:
				case TK_UNTIL: case TK_EOS:
					return true;
			  
				default:
					return false;
			}
		}

		private function check(c:int):void
		{
			if (this._token != c)
			{
				error_expected(c);
			}
		}

	    /**
	     * @param what   the token that is intended to end the match.
	     * @param who    the token that begins the match.
	     * @param where  the line number of <var>what</var>.
	     */
		private function check_match(what:int, who:int, where:int):void
			//throws IOException
		{
			if (!testnext(what))
			{
				if (where == this._linenumber)
				{
					error_expected(what);
				}
				else
				{
					xSyntaxerror("'" + xToken2str(what) + "' expected (to close '" +
						xToken2str(who) + "' at line " + where + ")");
				}
			}
		}

		private function close_func():void
		{
			removevars(0);
			this._fs.kRet(0, 0);  // final return;
			this._fs.close();
			// :todo: check this is a valid assertion to make
			//# assert fs != fs.prev
			this._fs = this._fs.prev;
		}
		
		public static function opcode_name(op:int):String
		{
			switch (op)
			{
				case Lua.OP_MOVE: 
					return "MOVE";
		  
				case Lua.OP_LOADK: 
					return "LOADK";
		  
				case Lua.OP_LOADBOOL: 
					return "LOADBOOL";
		  
				case Lua.OP_LOADNIL: 
					return "LOADNIL";
		  
				case Lua.OP_GETUPVAL: 
					return "GETUPVAL";
		  
				case Lua.OP_GETGLOBAL: 
					return "GETGLOBAL";
		  
				case Lua.OP_GETTABLE: 
					return "GETTABLE";
		  
				case Lua.OP_SETGLOBAL: 
					return "SETGLOBAL";
		  
				case Lua.OP_SETUPVAL: 
					return "SETUPVAL";
		  
				case Lua.OP_SETTABLE: 
					return "SETTABLE";
		  
				case Lua.OP_NEWTABLE: 
					return "NEWTABLE";
		  
				case Lua.OP_SELF: 
					return "SELF";
		  
				case Lua.OP_ADD: 
					return "ADD";
		  
				case Lua.OP_SUB: 
					return "SUB";
		  
				case Lua.OP_MUL: 
					return "MUL";
		  
				case Lua.OP_DIV: 
					return "DIV";
		  
				case Lua.OP_MOD: 
					return "MOD";
		  
				case Lua.OP_POW: 
					return "POW";
		  
				case Lua.OP_UNM: 
					return "UNM";
		  
				case Lua.OP_NOT: 
					return "NOT";
		  
				case Lua.OP_LEN: 
					return "LEN";
		  
				case Lua.OP_CONCAT: 
					return "CONCAT";
		  
				case Lua.OP_JMP: 
					return "JMP";
		  
				case Lua.OP_EQ: 
					return "EQ";
		  
				case Lua.OP_LT: 
					return "LT";
		  
				case Lua.OP_LE: 
					return "LE";
		  
				case Lua.OP_TEST: 
					return "TEST";
		  
				case Lua.OP_TESTSET: 
					return "TESTSET";
		  
				case Lua.OP_CALL: 
					return "CALL";
		  
				case Lua.OP_TAILCALL: 
					return "TAILCALL";
		  
				case Lua.OP_RETURN: 
					return "RETURN";
		  
				case Lua.OP_FORLOOP: 
					return "FORLOOP";
		  
				case Lua.OP_FORPREP: 
					return "FORPREP";
		  
				case Lua.OP_TFORLOOP: 
					return "TFORLOOP";
		  
				case Lua.OP_SETLIST: 
					return "SETLIST";
		  
				case Lua.OP_CLOSE: 
					return "CLOSE";
		  
				case Lua.OP_CLOSURE: 
					return "CLOSURE";
		  
				case Lua.OP_VARARG: 
					return "VARARG";
		  
				default: 
					return "??"+op;
			}
		}

		private function codestring(e:Expdesc, s:String):void
		{
			e.init(Expdesc.VK, this._fs.kStringK(s));
		}

		private function checkname(e:Expdesc):void // throws IOException
		{
			codestring(e, str_checkname());
		}

		private function enterlevel():void
		{
			this._L.nCcalls++;
		}

		private function error_expected(tok:int):void
		{
			xSyntaxerror("'" + xToken2str(tok) + "' expected");
		}

		private function leavelevel():void
		{
			this._L.nCcalls--;
		}


		/** Equivalent to luaY_parser. */
		public static function parser(L:Lua, _in:Reader, name:String):Proto
		  //throws IOException
		{
			var ls:Syntax = new Syntax(L, _in, name);
			var fs:FuncState = new FuncState(ls);
			ls.open_func(fs);
			fs.f.isVararg = true;
			ls.xNext();
			ls.chunk();
			ls.check(TK_EOS);
			ls.close_func();
			//# assert fs.prev == null
			//# assert fs.f.nups == 0
			//# assert ls.fs == null
			return fs.f;
		}

		private function removevars(tolevel:int):void
		{
			// :todo: consider making a method in FuncState.
			while (this._fs.nactvar > tolevel)
			{
				this._fs.getlocvar(--this._fs.nactvar).endpc = this._fs.pc;
			}
		}

		private function singlevar(_var:Expdesc):void // throws IOException
		{
			var varname:String = str_checkname();
			if (singlevaraux(this._fs, varname, _var, true) == Expdesc.VGLOBAL)
			{
				_var.info = this._fs.kStringK(varname);
			}
		}

		private function singlevaraux(f:FuncState,
			n:String,
			_var:Expdesc,
			base:Boolean):int
		{
			if (f == null)      // no more levels?
			{
				_var.init(Expdesc.VGLOBAL, Lua.NO_REG);    // default is global variable
				return Expdesc.VGLOBAL;
			}
			else
			{
				var v:int = f.searchvar(n);
				if (v >= 0)
				{
					_var.init(Expdesc.VLOCAL, v);
					if (!base)
					{
						f.markupval(v);       // local will be used as an upval
					}
					return Expdesc.VLOCAL;
				}
				else    // not found at current level; try upper one
				{
					if (singlevaraux(f.prev, n, _var, false) == Expdesc.VGLOBAL)
					{
						return Expdesc.VGLOBAL;
					}
					_var.upval(indexupvalue(f, n, _var));     // else was LOCAL or UPVAL
					return Expdesc.VUPVAL;
				}
			}
		}

		private function str_checkname():String // throws IOException
		{
			check(TK_NAME);
			var s:String = this._tokenS;
			xNext();
			return s;
		}

		private function testnext(c:int):Boolean // throws IOException
		{
			if (this._token == c)
			{
				xNext();
				return true;
			}
			return false;
		}


		// GRAMMAR RULES

		private function chunk():void // throws IOException
		{
			// chunk -> { stat [';'] }
			var islast:Boolean = false;
			enterlevel();
			while (!islast && !block_follow(this._token))
			{
				islast = statement();
				testnext(';'.charCodeAt());
				//# assert fs.f.maxstacksize >= fs.freereg && fs.freereg >= fs.nactvar
				this._fs.freereg = this._fs.nactvar;
			}
			leavelevel();
		}

		private function constructor(t:Expdesc):void // throws IOException
		{
			// constructor -> ??
			var line:int = this._linenumber;
			var pc:int = this._fs.kCodeABC(Lua.OP_NEWTABLE, 0, 0, 0);
			var cc:ConsControl = new ConsControl(t) ;
			t.init(Expdesc.VRELOCABLE, pc);
			cc.v.init(Expdesc.VVOID, 0);        /* no value (yet) */
			this._fs.kExp2nextreg(t);  /* fix it at stack top (for gc) */
			checknext('{'.charCodeAt());
			do
			{
				//# assert cc.v.k == Expdesc.VVOID || cc.tostore > 0
				if (this._token == '}'.charCodeAt())
					break;
				closelistfield(cc);
				switch(String.fromCharCode(this._token))
				{
					case String.fromCharCode(TK_NAME):  /* may be listfields or recfields */
						xLookahead();
						if (this._lookahead != '='.charCodeAt())  /* expression? */
							listfield(cc);
						else
							recfield(cc);
						break;

					case '[':  /* constructor_item -> recfield */
						recfield(cc);
						break;

					default:  /* constructor_part -> listfield */
						listfield(cc);
						break;
				}
			} while (testnext(','.charCodeAt()) || testnext(';'.charCodeAt()));
			check_match('}'.charCodeAt(), '{'.charCodeAt(), line);
			lastlistfield(cc);
			var code:Array = this._fs.f.code; //int [] 
			code[pc] = Lua.SETARG_B(code[pc], oInt2fb(cc.na)); /* set initial array size */
			code[pc] = Lua.SETARG_C(code[pc], oInt2fb(cc.nh)); /* set initial table size */
		}

		private static function oInt2fb(x:int):int
		{
			var e:int = 0;  /* exponent */
			while (x < 0 || x >= 16)
			{
				x = (x+1) >>> 1;
				e++;
			}
			return (x < 8) ? x : (((e+1) << 3) | (x - 8));
		}

		private function recfield(cc:ConsControl):void  //throws IOException
		{
			/* recfield -> (NAME | `['exp1`]') = exp1 */
			var reg:int = this._fs.freereg;
			var key:Expdesc = new Expdesc();
			var val:Expdesc = new Expdesc();
			if (this._token == TK_NAME)
			{
				// yChecklimit(fs, cc.nh, MAX_INT, "items in a constructor");
				checkname(key);
			}
			else  /* token == '[' */
				yindex(key);
			cc.nh++;
			checknext('='.charCodeAt());
			this._fs.kExp2RK(key);
			expr(val);
			this._fs.kCodeABC(Lua.OP_SETTABLE, cc.t.info, this._fs.kExp2RK(key), this._fs.kExp2RK(val));
			this._fs.freereg = reg;  /* free registers */
		}

		private function lastlistfield(cc:ConsControl):void
		{
			if (cc.tostore == 0)
				return;
			if (hasmultret(cc.v.k))
			{
				this._fs.kSetmultret(cc.v);
				this._fs.kSetlist(cc.t.info, cc.na, Lua.MULTRET);
				cc.na--;  /* do not count last expression (unknown number of elements) */
			}
			else
			{
				if (cc.v.k != Expdesc.VVOID)
					this._fs.kExp2nextreg(cc.v);
				this._fs.kSetlist(cc.t.info, cc.na, cc.tostore);
			}
		}

		private function closelistfield(cc:ConsControl):void
		{
			if (cc.v.k == Expdesc.VVOID)
				return;  /* there is no list item */
			this._fs.kExp2nextreg(cc.v);
			cc.v.k = Expdesc.VVOID;
			if (cc.tostore == Lua.LFIELDS_PER_FLUSH)
			{
				this._fs.kSetlist(cc.t.info, cc.na, cc.tostore);  /* flush */
				cc.tostore = 0;  /* no more items pending */
			}
		}

		private function expr(v:Expdesc):void // throws IOException
		{
			subexpr(v, 0);
		}

		/** @return number of expressions in expression list. */
		private function explist1(v:Expdesc):int // throws IOException
		{
			// explist1 -> expr { ',' expr }
			var n:int = 1;  // at least one expression
			expr(v);
			while (testnext(','.charCodeAt()))
			{
				this._fs.kExp2nextreg(v);
				expr(v);
				++n;
			}
			return n;
		}

		private function exprstat():void // throws IOException
		{
			// stat -> func | assignment
			var v:LHSAssign = new LHSAssign() ;
			primaryexp(v.v);
			if (v.v.k == Expdesc.VCALL)      // stat -> func
			{
				this._fs.setargc(v.v, 1); // call statement uses no results
			}
			else      // stat -> assignment
			{
				v.prev = null;
				assignment(v, 1);
			}
		}

		/*
		** check whether, in an assignment to a local variable, the local variable
		** is needed in a previous assignment (to a table). If so, save original
		** local value in a safe place and use this safe copy in the previous
		** assignment.
		*/
		private function check_conflict(lh:LHSAssign, v:Expdesc):void
		{
			var extra:int = this._fs.freereg;  /* eventual position to save local variable */
			var conflict:Boolean = false ;
			for (; lh != null; lh = lh.prev)
			{
				if (lh.v.k == Expdesc.VINDEXED)
				{
					if (lh.v.info == v.info)    /* conflict? */
					{
					  conflict = true;
					  lh.v.info = extra;  /* previous assignment will use safe copy */
					}
					if (lh.v.aux == v.info)    /* conflict? */
					{
					  conflict = true;
					  lh.v.aux = extra;  /* previous assignment will use safe copy */
					}
				}
			}
			if (conflict)
			{
				this._fs.kCodeABC(Lua.OP_MOVE, this._fs.freereg, v.info, 0);  /* make copy */
				this._fs.kReserveregs(1);
			}
		}

		private function assignment(lh:LHSAssign, nvars:int):void // throws IOException
		{
			var e:Expdesc = new Expdesc() ;
			var kind:int = lh.v.k ;
			if (!(Expdesc.VLOCAL <= kind && kind <= Expdesc.VINDEXED))
				xSyntaxerror("syntax error");
			if (testnext(','.charCodeAt()))    /* assignment -> `,' primaryexp assignment */
			{
				var nv:LHSAssign = new LHSAssign();
				nv.init(lh); //TODO:
				primaryexp(nv.v);
				if (nv.v.k == Expdesc.VLOCAL)
					check_conflict(lh, nv.v);
				assignment(nv, nvars+1);
			}
			else    /* assignment -> `=' explist1 */
			{
				var nexps:int;
				checknext('='.charCodeAt());
				nexps = explist1(e);
				if (nexps != nvars)
				{
					adjust_assign(nvars, nexps, e);
					if (nexps > nvars)
						this._fs.freereg -= nexps - nvars;  /* remove extra values */
				}
				else
				{
					this._fs.kSetoneret(e);  /* close last expression */
					this._fs.kStorevar(lh.v, e);
					return;  /* avoid default */
				}
			}
			e.init(Expdesc.VNONRELOC, this._fs.freereg - 1);    /* default assignment */
			this._fs.kStorevar(lh.v, e);
		}


		private function funcargs(f:Expdesc):void // throws IOException
		{
			var args:Expdesc = new Expdesc();
			var line:int = this._linenumber;
			switch (String.fromCharCode(this._token))
			{
				case '(':         // funcargs -> '(' [ explist1 ] ')'
					if (line != lastline)
					{
						xSyntaxerror("ambiguous syntax (function call x new statement)");
					}
					xNext();
					if (this._token == ')'.charCodeAt())       // arg list is empty?
					{
						args.kind = Expdesc.VVOID;
					}
					else
					{
						explist1(args);
						this._fs.kSetmultret(args);
					}
					check_match(')'.charCodeAt(), '('.charCodeAt(), line);
					break;

				case '{':         // funcargs -> constructor
					constructor(args);
					break;

				case String.fromCharCode(TK_STRING):   // funcargs -> STRING
					codestring(args, this._tokenS);
					xNext();        // must use tokenS before 'next'
					break;

				default:
					xSyntaxerror("function arguments expected");
					return;
			}
			// assert (f.kind() == VNONRELOC);
			var nparams:int;
			var base:int = f.info;        // base register for call
			if (args.hasmultret())
			{
				nparams = Lua.MULTRET;     // open call
			}
			else
			{
				if (args.kind != Expdesc.VVOID)
				{
					this._fs.kExp2nextreg(args);  // close last argument
				}
				nparams = this._fs.freereg - (base+1);
			}
			f.init(Expdesc.VCALL, this._fs.kCodeABC(Lua.OP_CALL, base, nparams+1, 2));
			this._fs.kFixline(line);
			this._fs.freereg = base+1;        // call removes functions and arguments
						// and leaves (unless changed) one result.
		}

		private function prefixexp(v:Expdesc):void // throws IOException
		{
			// prefixexp -> NAME | '(' expr ')'
			switch (String.fromCharCode(this._token))
			{
				case '(':
					{
						var line:int = this._linenumber;
						xNext();
						expr(v);
						check_match(')'.charCodeAt(), '('.charCodeAt(), line);
						this._fs.kDischargevars(v);
						return;
					}
				
				case String.fromCharCode(TK_NAME):
					singlevar(v);
					return;
			  
				default:
					xSyntaxerror("unexpected symbol");
					return;
			}
		}

		private function primaryexp(v:Expdesc):void // throws IOException
		{
			// primaryexp ->
			//    prefixexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs }
			prefixexp(v);
			while (true)
			{
				switch (String.fromCharCode(this._token))
				{
					case '.':  /* field */
						field(v);
						break;

					case '[':  /* `[' exp1 `]' */
						{
							var key:Expdesc = new Expdesc();
							this._fs.kExp2anyreg(v);
							yindex(key);
							this._fs.kIndexed(v, key);
						}
						break;

					case ':':  /* `:' NAME funcargs */
						{
							var key2:Expdesc = new Expdesc() ;
							xNext();
							checkname(key2);
							this._fs.kSelf(v, key2);
							funcargs(v);
						}
						break;

					case '(':
					case String.fromCharCode(TK_STRING):
					case '{':     // funcargs
						this._fs.kExp2nextreg(v);
						funcargs(v);
						break;

					default:
						return;
				}
			}
		}

		private function retstat():void // throws IOException
		{
			// stat -> RETURN explist
			xNext();    // skip RETURN
			// registers with returned values (first, nret)
			var first:int = 0;
			var nret:int;
			if (block_follow(this._token) || this._token == ';'.charCodeAt())
			{
				// return no values
				first = 0;
				nret = 0;
			}
			else
			{
				var e:Expdesc = new Expdesc();
				nret = explist1(e);
				if (hasmultret(e.k))
				{
					this._fs.kSetmultret(e);
					if (e.k == Expdesc.VCALL && nret == 1)    /* tail call? */
					{
						this._fs.setcode(e, Lua.SET_OPCODE(this._fs.getcode(e), Lua.OP_TAILCALL));
						//# assert Lua.ARGA(fs.getcode(e)) == fs.nactvar
					}
					first = this._fs.nactvar;
					nret = Lua.MULTRET;  /* return all values */
				}
				else
				{
					if (nret == 1)          // only one single value?
					{
						first = this._fs.kExp2anyreg(e);
					}
					else
					{
						this._fs.kExp2nextreg(e);  /* values must go to the `stack' */
						first = this._fs.nactvar;  /* return all `active' values */
						//# assert nret == fs.freereg - first
					}
				}
			}
			this._fs.kRet(first, nret);
		}

		private function simpleexp(v:Expdesc):void // throws IOException
		{
			// simpleexp -> NUMBER | STRING | NIL | true | false | ... |
			//              constructor | FUNCTION body | primaryexp
			switch (this._token)
			{
				case TK_NUMBER:
					v.init(Expdesc.VKNUM, 0);
					v.nval = this._tokenR;
					break;

				case TK_STRING:
					codestring(v, this._tokenS);
					break;

				case TK_NIL:
					v.init(Expdesc.VNIL, 0);
					break;

				case TK_TRUE:
					v.init(Expdesc.VTRUE, 0);
					break;

				case TK_FALSE:
					v.init(Expdesc.VFALSE, 0);
					break;

				case TK_DOTS:  /* vararg */
					if (!this._fs.f.isVararg)
						xSyntaxerror("cannot use \"...\" outside a vararg function");
					v.init(Expdesc.VVARARG, this._fs.kCodeABC(Lua.OP_VARARG, 0, 1, 0));
					break;

				case '{'.charCodeAt():   /* constructor */
					constructor(v);
					return;

				case TK_FUNCTION:
					xNext();
					body(v, false, this._linenumber);
					return;

				default:
					primaryexp(v);
					return;
			}
			xNext();
		}

		private function statement():Boolean //throws IOException
		{
			var line:int = this._linenumber;
			switch (this._token)
			{
				case TK_IF:   // stat -> ifstat
					ifstat(line);
					return false;

				case TK_WHILE:  // stat -> whilestat
					whilestat(line);
					return false;

				case TK_DO:       // stat -> DO block END
					xNext();         // skip DO
					block();
					check_match(TK_END, TK_DO, line);
					return false;

				case TK_FOR:      // stat -> forstat
					forstat(line);
					return false;

				case TK_REPEAT:   // stat -> repeatstat
					repeatstat(line);
					return false;

				case TK_FUNCTION:
					funcstat(line); // stat -> funcstat
					return false;

				case TK_LOCAL:    // stat -> localstat
					xNext();         // skip LOCAL
					if (testnext(TK_FUNCTION))  // local function?
						localfunc();
					else
						localstat();
					return false;

				case TK_RETURN:
					retstat();
					return true;  // must be last statement

				case TK_BREAK:  // stat -> breakstat
					xNext();       // skip BREAK
					breakstat();
					return true;  // must be last statement
				
				default:
					exprstat();
					return false;
			}
		}

		// grep "ORDER OPR" if you change these enums.
		// default access so that FuncState can access them.
		public static const OPR_ADD:int = 0;
		public static const OPR_SUB:int = 1;
		public static const OPR_MUL:int = 2;
		public static const OPR_DIV:int = 3;
		public static const OPR_MOD:int = 4;
		public static const OPR_POW:int = 5;
		public static const OPR_CONCAT:int = 6;
		public static const OPR_NE:int = 7;
		public static const OPR_EQ:int = 8;
		public static const OPR_LT:int = 9;
		public static const OPR_LE:int = 10;
		public static const OPR_GT:int = 11;
		public static const OPR_GE:int = 12;
		public static const OPR_AND:int = 13;
		public static const OPR_OR:int = 14;
		public static const OPR_NOBINOPR:int = 15;

		public static const OPR_MINUS:int = 0;
		public static const OPR_NOT:int = 1;
		public static const OPR_LEN:int = 2;
		public static const OPR_NOUNOPR:int = 3;

		/** Converts token into binary operator.  */
		private static function getbinopr(op:int):int
		{
			switch (String.fromCharCode(op))
			{
				case '+': 
					return OPR_ADD;
			  
				case '-': 
					return OPR_SUB;
			  
				case '*': 
					return OPR_MUL;
			  
				case '/': 
					return OPR_DIV;
			  
				case '%': 
					return OPR_MOD;
			  
				case '^': 
					return OPR_POW;
			  
				case String.fromCharCode(TK_CONCAT): 
					return OPR_CONCAT;
			  
				case String.fromCharCode(TK_NE): 
					return OPR_NE;
			  
				case String.fromCharCode(TK_EQ): 
					return OPR_EQ;
			  
				case '<': 
					return OPR_LT;
			  
				case String.fromCharCode(TK_LE): 
					return OPR_LE;
			  
				case '>': 
					return OPR_GT;
			  
				case String.fromCharCode(TK_GE): 
					return OPR_GE;
			  
				case String.fromCharCode(TK_AND): 
					return OPR_AND;
			  
				case String.fromCharCode(TK_OR): 
					return OPR_OR;
			  
				default: 
					return OPR_NOBINOPR;
			}
		}

		private static function getunopr(op:int):int
		{
			switch (String.fromCharCode(op))
			{
				case String.fromCharCode(TK_NOT): 
					return OPR_NOT;
				
				case '-': 
					return OPR_MINUS;
				
				case '#': 
					return OPR_LEN;
				
				default: 
					return OPR_NOUNOPR;
			}
		}


		// ORDER OPR
		/**
		* Priority table.  left-priority of an operator is
		* <code>priority[op][0]</code>, its right priority is
		* <code>priority[op][1]</code>.  Please do not modify this table.
		*/
		private static var PRIORITY:Array = //new int[][]
		[
			[6, 6], [6, 6], [7, 7], [7, 7], [7, 7],     // + - * / %
			[10, 9], [5, 4],                // power and concat (right associative)
			[3, 3], [3, 3],                 // equality and inequality
			[3, 3], [3, 3], [3, 3], [3, 3], // order
			[2, 2], [1, 1]                  // logical (and/or)
		];

		/** Priority for unary operators. */
		private static const UNARY_PRIORITY:int = 8;

		/**
		 * Operator precedence parser.
		 * <code>subexpr -> (simpleexp) | unop subexpr) { binop subexpr }</code>
		 * where <var>binop</var> is any binary operator with a priority
		 * higher than <var>limit</var>.
		 */
		private function subexpr(v:Expdesc, limit:int):int // throws IOException
		{
			enterlevel();
			var uop:int = getunopr(this._token);
			if (uop != OPR_NOUNOPR)
			{
				xNext();
				subexpr(v, UNARY_PRIORITY);
				this._fs.kPrefix(uop, v);
			}
			else
			{
				simpleexp(v);
			}
			// expand while operators have priorities higher than 'limit'
			var op:int = getbinopr(this._token);
			while (op != OPR_NOBINOPR && PRIORITY[op][0] > limit)
			{
				var v2:Expdesc = new Expdesc();
				xNext();
				this._fs.kInfix(op, v);
				// read sub-expression with higher priority
				var nextop:int = subexpr(v2, PRIORITY[op][1]);
				this._fs.kPosfix(op, v, v2);
				op = nextop;
			}
			leavelevel();
			return op;
		}

		private function enterblock(f:FuncState, bl:BlockCnt, isbreakable:Boolean):void
		{
			bl.breaklist = FuncState.NO_JUMP;
			bl.isbreakable = isbreakable;
			bl.nactvar = f.nactvar;
			bl.upval = false;
			bl.previous = f.bl;
			f.bl = bl;
			//# assert f.freereg == f.nactvar
		}

		private function leaveblock(f:FuncState):void
		{
			var bl:BlockCnt = f.bl;
			f.bl = bl.previous;
			removevars(bl.nactvar);
			if (bl.upval)
				f.kCodeABC(Lua.OP_CLOSE, bl.nactvar, 0, 0);
			/* loops have no body */
			//# assert (!bl.isbreakable) || (!bl.upval)
			//# assert bl.nactvar == f.nactvar
			f.freereg = f.nactvar;  /* free registers */
			f.kPatchtohere(bl.breaklist);
		}


		/*
		** {======================================================================
		** Rules for Statements
		** =======================================================================
		*/


		private function block():void // throws IOException
		{
			/* block -> chunk */
			var bl:BlockCnt = new BlockCnt() ;
			enterblock(this._fs, bl, false);
			chunk();
			//# assert bl.breaklist == FuncState.NO_JUMP
			leaveblock(this._fs);
		}

		private function breakstat():void
		{
			var bl:BlockCnt = this._fs.bl;
			var upval:Boolean = false;
			while (bl != null && !bl.isbreakable)
			{
				//TODO:||=
				upval ||= bl.upval;
				bl = bl.previous;
			}
			if (bl == null)
				xSyntaxerror("no loop to break");
			if (upval)
				this._fs.kCodeABC(Lua.OP_CLOSE, bl.nactvar, 0, 0);
			bl.breaklist = this._fs.kConcat(bl.breaklist, this._fs.kJump());
		}

		private function funcstat(line:int):void //throws IOException
		{
			/* funcstat -> FUNCTION funcname body */
			var b:Expdesc = new Expdesc() ;
			var v:Expdesc = new Expdesc() ;
			xNext();  /* skip FUNCTION */
			var needself:Boolean = funcname(v);
			body(b, needself, line);
			this._fs.kStorevar(v, b);
			this._fs.kFixline(line);  /* definition `happens' in the first line */
		}

		private function checknext(c:int):void // throws IOException
		{
			check(c);
			xNext();
		}

		private function parlist():void // throws IOException
		{
			/* parlist -> [ param { `,' param } ] */
			var f:Proto = this._fs.f;
			var nparams:int = 0;
			if (this._token != ')'.charCodeAt())    /* is `parlist' not empty? */
			{
				do
				{
					switch (this._token)
					{
						case TK_NAME:    /* param -> NAME */
							{
								new_localvar(str_checkname(), nparams++);
								break;
							}
					  
						case TK_DOTS:    /* param -> `...' */
							{
								xNext();
								f.isVararg = true;
								break;
							}
							
						default: 
							xSyntaxerror("<name> or '...' expected");
					}
				} while ((!f.isVararg) && testnext(','.charCodeAt()));
			}
			adjustlocalvars(nparams);
			f.numparams = this._fs.nactvar ; /* VARARG_HASARG not now used */
			this._fs.kReserveregs(this._fs.nactvar);  /* reserve register for parameters */
		}


		private function getlocvar(i:int):LocVar
		{
			var fstate:FuncState = this._fs;
			return fstate.f.locvars [fstate.actvar[i]] ;
		}

		private function adjustlocalvars(nvars:int):void
		{
			this._fs.nactvar += nvars;
			for (; nvars != 0; nvars--)
			{
				getlocvar(this._fs.nactvar - nvars).startpc = this._fs.pc;
			}
		}

		private function new_localvarliteral(v:String, n:int):void
		{
			new_localvar(v, n) ;
		}

		private function errorlimit(limit:int, what:String):void
		{
			var msg:String = this._fs.f.linedefined == 0 ?
				"main function has more than " + limit + " " + what :
				"function at line " + this._fs.f.linedefined + " has more than " + limit + " " + what;
			xLexerror(msg, 0);
		}


		private function yChecklimit(v:int, l:int, m:String):void
		{
			if (v > l)
				errorlimit(l,m);
		}

		private function new_localvar(name:String, n:int):void
		{
			yChecklimit(this._fs.nactvar + n + 1, Lua.MAXVARS, "local variables");
			this._fs.actvar[this._fs.nactvar + n] = registerlocalvar(name) as int;
		}

		private function registerlocalvar(varname:String):int
		{
			var f:Proto = this._fs.f;
			f.ensureLocvars(this._L, this._fs.nlocvars, /*Short*/int.MAX_VALUE) ; //TODO:
			(f.locvars[this._fs.nlocvars] as LocVar).varname = varname;
			return this._fs.nlocvars++;
		}


		private function body(e:Expdesc, needself:Boolean, line:int):void // throws IOException
		{
			/* body ->  `(' parlist `)' chunk END */
			var new_fs:FuncState = new FuncState(this);
			open_func(new_fs);
			new_fs.f.linedefined = line;
			checknext('('.charCodeAt());
			if (needself)
			{
				new_localvarliteral("self", 0);
				adjustlocalvars(1);
			}
			parlist();
			checknext(')'.charCodeAt());
			chunk();
			new_fs.f.lastlinedefined = this._linenumber;
			check_match(TK_END, TK_FUNCTION, line);
			close_func();
			pushclosure(new_fs, e);
		}

		private function UPVAL_K(upvaldesc:int):int
		{
			return (upvaldesc >>> 8) & 0xFF ;
		}
		
		private function UPVAL_INFO(upvaldesc:int):int
		{
			return upvaldesc & 0xFF ;
		}
		
		private function UPVAL_ENCODE(k:int, info:int):int
		{
			//# assert (k & 0xFF) == k && (info & 0xFF) == info
			return ((k & 0xFF) << 8) | (info & 0xFF) ;
		}


		private function pushclosure(func:FuncState, v:Expdesc):void
		{
			var f:Proto = this._fs.f;
			f.ensureProtos(this._L, this._fs.np) ;
			var ff:Proto = func.f ;
			f.p[this._fs.np++] = ff;
			v.init(Expdesc.VRELOCABLE, this._fs.kCodeABx(Lua.OP_CLOSURE, 0, this._fs.np - 1));
			for (var i:int = 0; i < ff.nups; i++)
			{
				var upvalue:int = func.upvalues[i] ;
				var o:int = (UPVAL_K(upvalue) == Expdesc.VLOCAL) ? Lua.OP_MOVE :
															 Lua.OP_GETUPVAL;
				this._fs.kCodeABC(o, 0, UPVAL_INFO(upvalue), 0);
			}
		}

		private function funcname(v:Expdesc):Boolean // throws IOException
		{
			/* funcname -> NAME {field} [`:' NAME] */
			var needself:Boolean = false;
			singlevar(v);
			while (this._token == '.'.charCodeAt())
				field(v);
			if (this._token == ':'.charCodeAt())
			{
				needself = true;
				field(v);
			}
			return needself;
		}

		private function field(v:Expdesc):void //throws IOException
		{
			/* field -> ['.' | ':'] NAME */
			var key:Expdesc = new Expdesc() ;
			this._fs.kExp2anyreg(v);
			xNext();  /* skip the dot or colon */
			checkname(key);
			this._fs.kIndexed(v, key);
		}

		private function repeatstat(line:int):void //throws IOException
		{
			/* repeatstat -> REPEAT block UNTIL cond */
			var repeat_init:int = this._fs.kGetlabel();
			var bl1:BlockCnt = new BlockCnt();
			var bl2:BlockCnt = new BlockCnt();
			enterblock(this._fs, bl1, true);  /* loop block */
			enterblock(this._fs, bl2, false);  /* scope block */
			xNext();  /* skip REPEAT */
			chunk();
			check_match(TK_UNTIL, TK_REPEAT, line);
			var condexit:int = cond();  /* read condition (inside scope block) */
			if (!bl2.upval)    /* no upvalues? */
			{
				leaveblock(this._fs);  /* finish scope */
				this._fs.kPatchlist(condexit, repeat_init);  /* close the loop */
			}
			else    /* complete semantics when there are upvalues */
			{
				breakstat();  /* if condition then break */
				this._fs.kPatchtohere(condexit);  /* else... */
				leaveblock(this._fs);  /* finish scope... */
				this._fs.kPatchlist(this._fs.kJump(), repeat_init);  /* and repeat */
			}
			leaveblock(this._fs);  /* finish loop */
		}

		private function cond():int // throws IOException
		{
			/* cond -> exp */
			var v:Expdesc = new Expdesc() ;
			expr(v);  /* read condition */
			if (v.k == Expdesc.VNIL)
				v.k = Expdesc.VFALSE;  /* `falses' are all equal here */
			this._fs.kGoiftrue(v);
			return v.f;
		}

		private function open_func(funcstate:FuncState):void
		{
			var f:Proto = new Proto();  /* registers 0/1 are always valid */
			f.init2(source, 2);
			funcstate.f = f;
			funcstate.ls = this;
			funcstate.L = this._L;

			funcstate.prev = this._fs;   /* linked list of funcstates */
			this._fs = funcstate;
		}

		private function localstat():void  // throws IOException
		{
			/* stat -> LOCAL NAME {`,' NAME} [`=' explist1] */
			var nvars:int = 0;
			var nexps:int;
			var e:Expdesc = new Expdesc();
			do
			{
				new_localvar(str_checkname(), nvars++);
			} while (testnext(','.charCodeAt()));
			if (testnext('='.charCodeAt()))
			{
				nexps = explist1(e);
			}
			else
			{
				e.k = Expdesc.VVOID;
				nexps = 0;
			}
			adjust_assign(nvars, nexps, e);
			adjustlocalvars(nvars);
		}

		private function forstat(line:int):void // throws IOException
		{
			/* forstat -> FOR (fornum | forlist) END */
			var bl:BlockCnt = new BlockCnt() ;
			enterblock(this._fs, bl, true);  /* scope for loop and control variables */
			xNext();  /* skip `for' */
			var varname:String = str_checkname();  /* first variable name */
			switch (String.fromCharCode(this._token))
			{
				case '=':
					fornum(varname, line);
					break;
			  
				case ',':
				case String.fromCharCode(TK_IN):
					forlist(varname);
					break;
			  
				default:
					xSyntaxerror("\"=\" or \"in\" expected");
			}
			check_match(TK_END, TK_FOR, line);
			leaveblock(this._fs);  /* loop scope (`break' jumps to this point) */
		}

		private function fornum(varname:String, line:int):void // throws IOException
		{
			/* fornum -> NAME = exp1,exp1[,exp1] forbody */
			var base:int = this._fs.freereg;
			new_localvarliteral("(for index)", 0);
			new_localvarliteral("(for limit)", 1);
			new_localvarliteral("(for step)", 2);
			new_localvar(varname, 3);
			checknext('='.charCodeAt());
			exp1();  /* initial value */
			checknext(','.charCodeAt());
			exp1();  /* limit */
			if (testnext(','.charCodeAt()))
				exp1();  /* optional step */
			else    /* default step = 1 */
			{
				this._fs.kCodeABx(Lua.OP_LOADK, this._fs.freereg, this._fs.kNumberK(1));
				this._fs.kReserveregs(1);
			}
			forbody(base, line, 1, true);
		}

		private function exp1():int // throws IOException
		{
			var e:Expdesc = new Expdesc();
			expr(e);
			var k:int = e.k;
			this._fs.kExp2nextreg(e);
			return k;
		}

		private function forlist(indexname:String):void // throws IOException
		{
			/* forlist -> NAME {,NAME} IN explist1 forbody */
			var e:Expdesc = new Expdesc() ;
			var nvars:int = 0;
			var base:int = this._fs.freereg;
			/* create control variables */
			new_localvarliteral("(for generator)", nvars++);
			new_localvarliteral("(for state)", nvars++);
			new_localvarliteral("(for control)", nvars++);
			/* create declared variables */
			new_localvar(indexname, nvars++);
			while (testnext(','.charCodeAt()))
				new_localvar(str_checkname(), nvars++);
			checknext(TK_IN);
			var line:int = this._linenumber;
			adjust_assign(3, explist1(e), e);
			this._fs.kCheckstack(3);  /* extra space to call generator */
			forbody(base, line, nvars - 3, false);
		}

		private function forbody(base:int, line:int, nvars:int, isnum:Boolean):void
		  //throws IOException
		{
			/* forbody -> DO block */
			var bl:BlockCnt = new BlockCnt() ;
			adjustlocalvars(3);  /* control variables */
			checknext(TK_DO);
			var prep:int = isnum ? this._fs.kCodeAsBx(Lua.OP_FORPREP, base, FuncState.NO_JUMP) : this._fs.kJump();
			enterblock(this._fs, bl, false);  /* scope for declared variables */
			adjustlocalvars(nvars);
			this._fs.kReserveregs(nvars);
			block();
			leaveblock(this._fs);  /* end of scope for declared variables */
			this._fs.kPatchtohere(prep);
			var endfor:int = isnum ?
				this._fs.kCodeAsBx(Lua.OP_FORLOOP, base, FuncState.NO_JUMP) :
				this._fs.kCodeABC(Lua.OP_TFORLOOP, base, 0, nvars);
			this._fs.kFixline(line);  /* pretend that `OP_FOR' starts the loop */
			this._fs.kPatchlist((isnum ? endfor : this._fs.kJump()), prep + 1);
		}

		private function ifstat(line:int):void // throws IOException
		{
			/* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
			var escapelist:int = FuncState.NO_JUMP;
			var flist:int = test_then_block();  /* IF cond THEN block */
			while (this._token == TK_ELSEIF)
			{
				escapelist = this._fs.kConcat(escapelist, this._fs.kJump());
				this._fs.kPatchtohere(flist);
				flist = test_then_block();  /* ELSEIF cond THEN block */
			}
			if (this._token == TK_ELSE)
			{
				escapelist = this._fs.kConcat(escapelist, this._fs.kJump());
				this._fs.kPatchtohere(flist);
				xNext();  /* skip ELSE (after patch, for correct line info) */
				block();  /* `else' part */
			}
			else
				escapelist = this._fs.kConcat(escapelist, flist);
			
			this._fs.kPatchtohere(escapelist);
			check_match(TK_END, TK_IF, line);
		}

		private function test_then_block():int // throws IOException
		{
			/* test_then_block -> [IF | ELSEIF] cond THEN block */
			xNext();  /* skip IF or ELSEIF */
			var condexit:int = cond();
			checknext(TK_THEN);
			block();  /* `then' part */
			return condexit;
		}

		private function whilestat(line:int):void // throws IOException
		{
			/* whilestat -> WHILE cond DO block END */
			var bl:BlockCnt = new BlockCnt() ;
			xNext();  /* skip WHILE */
			var whileinit:int = this._fs.kGetlabel();
			var condexit:int = cond();
			enterblock(this._fs, bl, true);
			checknext(TK_DO);
			block();
			this._fs.kPatchlist(this._fs.kJump(), whileinit);
			check_match(TK_END, TK_WHILE, line);
			leaveblock(this._fs);
			this._fs.kPatchtohere(condexit);  /* false conditions finish the loop */
		}

		private static function hasmultret(k:int):Boolean
		{
			return k == Expdesc.VCALL || k == Expdesc.VVARARG ;
		}

		private function adjust_assign(nvars:int, nexps:int, e:Expdesc):void
		{
			var extra:int = nvars - nexps;
			if (hasmultret(e.k))
			{
				extra++;  /* includes call itself */
				if (extra < 0)
					extra = 0;
				this._fs.kSetreturns(e, extra);  /* last exp. provides the difference */
				if (extra > 1)
					this._fs.kReserveregs(extra-1);
			}
			else
			{
				if (e.k != Expdesc.VVOID)
					this._fs.kExp2nextreg(e);  /* close last expression */
				if (extra > 0)
				{
					var reg:int = this._fs.freereg;
					this._fs.kReserveregs(extra);
					this._fs.kNil(reg, extra);
				}
			}
		}

		private function localfunc():void // throws IOException
		{
			var b:Expdesc = new Expdesc();
			new_localvar(str_checkname(), 0);
			var v:Expdesc = new Expdesc();
			v.init(Expdesc.VLOCAL, this._fs.freereg);
			this._fs.kReserveregs(1);
			adjustlocalvars(1);
			body(b, false, this._linenumber);
			this._fs.kStorevar(v, b);
			/* debug information will only see the variable after this point! */
			this._fs.getlocvar(this._fs.nactvar - 1).startpc = this._fs.pc;
		}

		private function yindex(v:Expdesc):void  // throws IOException
		{
			/* index -> '[' expr ']' */
			xNext();  /* skip the '[' */
			expr(v);
			this._fs.kExp2val(v);
			checknext(']'.charCodeAt());
		}

		public function xLookahead():void  // throws IOException
		{
			//# assert lookahead == TK_EOS
			this._lookahead = llex();
			this._lookaheadR = this._semR ;
			this._lookaheadS = this._semS ;
		}

		private function listfield(cc:ConsControl):void // throws IOException
		{
			expr(cc.v);
			yChecklimit(cc.na, Lua.MAXARG_Bx, "items in a constructor");
			cc.na++;
			cc.tostore++;
		}

		private function indexupvalue(funcstate:FuncState, name:String, v:Expdesc):int
		{
			var f:Proto = funcstate.f;
			var oldsize:int = f.sizeupvalues;
			for (var i:int = 0; i < f.nups; i++)
			{
				var entry:int = funcstate.upvalues[i];
				if (UPVAL_K(entry) == v.k && UPVAL_INFO(entry) == v.info)
				{
					//# assert name.equals(f.upvalues[i])
					return i;
				}
			}
			/* new one */
			yChecklimit(f.nups + 1, Lua.MAXUPVALUES, "upvalues");
			f.ensureUpvals(this._L, f.nups) ;
			f.upvalues[f.nups] = name;
			//# assert v.k == Expdesc.VLOCAL || v.k == Expdesc.VUPVAL
			funcstate.upvalues[f.nups] = UPVAL_ENCODE(v.k, v.info) ;
			return f.nups++;
		}
		
		//新增
		public function get L():Lua
		{
			return this._L;
		}
	}
}