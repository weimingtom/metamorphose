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
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	import com.iteye.weimingtom.metamorphose.java.Character;
	import com.iteye.weimingtom.metamorphose.java.NumberFormatException;
	
	public final class FormatItem
	{
		private var _L:Lua;
		private var _left:Boolean; // '-' flag
		private var _sign:Boolean; // '+' flag
		private var _space:Boolean; // ' ' flag
		private var _alt:Boolean;  // '#' flag
		private var _zero:Boolean; // '0' flag
		private var _width:int;    // minimum field width
		private	var _precision:int = -1;   // precision, -1 when no precision specified.
		private var _type:uint;    // the type of the conversion
		private var _length:int;   // length of the format item in the format string.

		/**
		 * Character used in formatted output when %e or %g format is used.
		 */
		public static var E_LOWER:uint = 'E'.charCodeAt();
		/**
		 * Character used in formatted output when %E or %G format is used.
		 */
		public static var E_UPPER:uint = 'E'.charCodeAt();
		
		/**
		 * Parse a format item (starting from after the <code>L_ESC</code>).
		 * If you promise that there won't be any format errors, then
		 * <var>L</var> can be <code>null</code>.
		 */
		public function FormatItem(L:Lua, s:String)
		{
			this._L = L;
			var i:int = 0;
			var l:int = s.length;
// parse flags
flag:
			while (true)
			{
				if (i >=l )
					L.error("invalid format");
				switch (s.charAt(i))
				{
					case '-':
						this._left = true;
						break;
						
					case '+':
						this._sign = true;
						break;
						
					case ' ':
						this._space = true;
						break;
					
					case '#':
						this._alt = true;
						break;
					
					case '0':
						this._zero = true;
						break;
					
					default:
						break flag;
				}
				++i;
			} /* flag */
			// parse width
			var widths:int = i;       // index of start of width specifier
			while (true)
			{
				if (i >= l)
					this._L.error("invalid format");
				if (Syntax.isdigit(s.charCodeAt(i))) //TODO:
					++i;
				else
					break;
			}
			if (widths < i)
			{
				try
				{
					this._width = int(s.substring(widths, i)); //TODO:
				}
				catch (e_:Error)
				{
					trace(e_.getStackTrace());
				}
			}
			// parse precision
			if (s.charAt(i) == '.')
			{
				++i;
				var precisions:int = i; // index of start of precision specifier
				while (true)
				{
					if (i >= l)
						L.error("invalid format");
					if (Syntax.isdigit(s.charCodeAt(i))) //TODO:
						++i;
					else
						break;
				}
				if (precisions < i)
				{
					try
					{
						this._precision = int(s.substring(precisions, i)); //TODO:
					}
					catch (e_:NumberFormatException)
					{
						trace(e_.getStackTrace());
					}
				}
			}
			switch (s.charAt(i))
			{
				case 'c':
				case 'd': case 'i':
				case 'o': case 'u': case 'x': case 'X':
				case 'e': case 'E': case 'f': case 'g': case 'G':
				case 'q':
				case 's':
					this._type = s.charCodeAt(i);
					length = i + 1;
					return;
			}
			this._L.error("invalid option to 'format'");
		}

		public function get length():int 
		{
			return _length;
		}
		
		public function set length(length:int):void 
		{
			_length = length;
		}
		
		public function get type():int 
		{
			return _type;
		}

		public function set type(type:int):void 
		{
			_type = type;
		}
		
		/**
		 * Format the converted string according to width, and left.
		 * zero padding is handled in either {@link FormatItem#formatInteger}
		 * or {@link FormatItem#formatFloat}
		 * (and width is fixed to 0 in such cases).  Therefore we can ignore
		 * zero.
		 */
		private function format(b:StringBuffer, s:String):void
		{
			var l:int = s.length;
			if (l >= this._width)
			{
				b.appendString(s);
				return;
			}
			var pad:StringBuffer = new StringBuffer();
			while (l < this._width)
			{
				pad.append(' '.charCodeAt());
				++l;
			}
			if (this._left)
			{
				b.appendString(s);
				b.appendStringBuffer(pad);
			}	
			else
			{
				b.appendStringBuffer(pad);
				b.appendString(s);
			}
		}
		
		// All the format* methods take a StringBuffer and append the
		// formatted representation of the value to it.
		// Sadly after a format* method has been invoked the object is left in
		// an unusable state and should not be used again.
			
		public function formatChar(b:StringBuffer, c:uint):void 
		{
			var s:String = String.fromCharCode(c); //TODO:
			format(b, s);
		}

		public function formatInteger(b:StringBuffer, i:Number):void
		{
			// :todo: improve inefficient use of implicit StringBuffer
			
			if (this._left)
				this._zero = false;
			if (this._precision >= 0)
				this._zero = false;

			var radix:int = 10;
			switch (String.fromCharCode(type))
			{
				case 'o':
					radix = 8;
					break;
			  
				case 'd': case 'i': case 'u':
					radix = 10;
					break;
			  
				case 'x': case 'X':
					radix = 16;
					break;
			  
				default:
					this._L.error("invalid format");
			}
			var s:String  = i.toString(radix);//Long.toString(i, radix);
			if (this._type == 'X'.charCodeAt())
				s = s.toUpperCase();
			if (this._precision == 0 && s == "0")
				s = "";
				
			// form a prefix by strippping possible leading '-',
			// pad to precision,
			// add prefix,
			// pad to width.
			// extra wart: padding with '0' is implemented using precision
			// because this makes handling the prefix easier.
			var prefix:String = "";
			if (s.substr(0, 1) == "-")
			{
				prefix = "-";
				s = s.substring(1);
			}
			if (this._alt && radix == 16)
				prefix = "0x";
			if (prefix == "")
			{
				if (this._sign)
					prefix = "+";
				else if (this._space)
					prefix = " ";
			}
			if (this._alt && radix == 8 && s.substr(0, 1) != "0")
				s = "0" + s;
			var l:int = s.length;
			if (this._zero)
			{
				this._precision = this._width - prefix.length;
				this._width = 0;
			}
			if (l < this._precision)
			{
				var p:StringBuffer = new StringBuffer();
				while (l < this._precision)
				{
					p.append('0'.charCodeAt());
					++l;
				}
				p.appendString(s);
				s = p.toString();
			}
			s = prefix + s;
			format(b, s);
		}
			
		public function formatFloat(b:StringBuffer, d:Number):void
		{
			switch (String.fromCharCode(this._type))
			{
				case 'g': case 'G':
					formatFloatG(b, d);
					return;
			  
				case 'f':
					formatFloatF(b, d);
					return;
			  
				case 'e': case 'E':
					formatFloatE(b, d);
					return;
			}
		}

		private function formatFloatE(b:StringBuffer, d:Number):void
		{
			var s:String = formatFloatRawE(d);
			format(b, s);
		}

		/**
		 * Returns the formatted string for the number without any padding
		 * (which can be added by invoking {@link FormatItem#format} later).
		 */
		private function formatFloatRawE(d:Number):String 
		{
			var m:Number = Math.abs(d);
			var offset:int = 0;
			if (m >= 1e-3 && m < 1e7)
			{
				d *= 1e10;
				offset = 10;
			}
			var s:String = d.toPrecision(this._precision);//String(d); //FIXME:整数转浮点问题
			var t:StringBuffer = new StringBuffer(s);
			var e:int;      // Exponent value
			if (d == 0)
			{
				e = 0;
			}
			else
			{
				var ei:int = s.indexOf('E');
				e = int(s.substring(ei+1));
				t._delete(ei, int.MAX_VALUE); //TODO:
			}
			
			precisionTrim(t);

			e -= offset;
			if (Character.isLowerCase(type))
			{
				t.append(E_LOWER);
			}
			else
			{
				t.append(E_UPPER);
			}
			if (e >= 0)
			{
				t.append('+'.charCodeAt());
			}
			t.appendString(String(e)); //TODO:

			zeroPad(t);
			return t.toString();
		}
		
		private function formatFloatF(b:StringBuffer, d:Number):void
		{
			var s:String = formatFloatRawF(d);
			format(b, s);
		}
			
		/**
		 * Returns the formatted string for the number without any padding
		 * (which can be added by invoking {@link FormatItem#format} later).
		 */
		private function formatFloatRawF(d:Number):String
		{
			//toPrecision
			var s:String = String(d);//d.toPrecision(this._precision);//String(d); //FIXME:整数转字符串会丢失小数点后1位精度
			if (d % 1 === 0)
			{
				s = d.toFixed(1);
			}
			var t:StringBuffer = new StringBuffer(s);

			var di:int = s.indexOf('.');
			var ei:int = s.indexOf('E');
			if (ei >= 0)
			{
				t._delete(ei, int.MAX_VALUE); //TODO:
				var e:int = int(s.substring(ei+1));
				
				var z:StringBuffer = new StringBuffer();
				for (var i:int = 0; i < Math.abs(e); ++i)
				{
					z.append('0'.charCodeAt());
				}
				
				if (e > 0)
				{
					t.deleteCharAt(di);
					t.appendStringBuffer(z);
					t.insert(di+e, '.'.charCodeAt());
				}
				else
				{
					t.deleteCharAt(di);
					var at:int = t.charAt(0) == '-'.charCodeAt() ? 1 : 0;
					t.insertStringBuffer(at, z);
					t.insert(di, '.'.charCodeAt());
				}
			}

			precisionTrim(t);
			zeroPad(t);

			return t.toString();
		}

		private function formatFloatG(b:StringBuffer, d:Number):void
		{
			if (this._precision == 0)
			{
				this._precision = 1;
			}
			if (this._precision < 0)
			{
				this._precision = 6;
			}
			var s:String;
			// Decide whether to use %e or %f style.
			var m:Number = Math.abs(d);
			if (m == 0)
			{
				// :todo: Could test for -0 and use "-0" appropriately.
				s = "0";
			}
			else if (m < 1e-4 || m >= Lua.iNumpow(10, this._precision))
			{
				// %e style
				--this._precision;
				s = formatFloatRawE(d);
				var di:int = s.indexOf('.');
				if (di >= 0)
				{
					// Trim trailing zeroes from fractional part
					var ei:int = s.indexOf('E');
					if (ei < 0)
					{
						ei = s.indexOf('e');
					}
					var i:int = ei - 1;
					while (s.charAt(i) == '0')
					{
						--i;
					}
					if (s.charAt(i) != '.')
					{
						++i;
					}
					var a:StringBuffer = new StringBuffer(s);
					a._delete(i, ei); //TODO:
					s = a.toString();
				}
			}
			else
			{
				// %f style
				// For %g precision specifies the number of significant digits,
				// for %f precision specifies the number of fractional digits.
				// There is a problem because it's not obvious how many fractional
				// digits to format, it could be more than precision
				// (when .0001 <= m < 1) or it could be less than precision
				// (when m >= 1).
				// Instead of trying to work out the correct precision to use for
				// %f formatting we use a worse case to get at least all the
				// necessary digits, then we trim using string editing.  The worst
				// case is that 3 zeroes come after the decimal point before there
				// are any significant digits.
				// Save the required number of significant digits
				var required:int = this._precision;
				this._precision += 3;
				s = formatFloatRawF(d);
				var fsd:int = 0;      // First Significant Digit
				while (s.charAt(fsd) == '0' || s.charAt(fsd) == '.')
				{
					++fsd;
				}
				// Note that all the digits to the left of the decimal point in
				// the formatted number are required digits (either significant
				// when m >= 1 or 0 when m < 1).  We know this because otherwise 
				// m >= (10**precision) and so formatting falls under the %e case.
				// That means that we can always trim the string at fsd+required
				// (this will remove the decimal point when m >=
				// (10**(precision-1)).
				var a2:StringBuffer = new StringBuffer(s);
				a2._delete(fsd+required, int.MAX_VALUE); //TODO:
				if (s.indexOf('.') < a2.length())
				{
					// Trim trailing zeroes
					var i2:int = a2.length() - 1;
					while (a2.charAt(i2) == '0'.charCodeAt())
					{
						a2.deleteCharAt(i2);
						--i2;
					}
					if (a2.charAt(i2) == '.'.charCodeAt())
					{
						a2.deleteCharAt(i2);
					}
				}
				s = a2.toString();
			}
			format(b, s);
		}
			
		public function formatString(b:StringBuffer, s:String):void
		{
			var p:String = s;

			if (this._precision >= 0 && this._precision < s.length)
			{
				p = s.substring(0, this._precision);
			}
			format(b, p);
		}
		
		private function precisionTrim(t:StringBuffer):void
		{
			if (this._precision < 0)
			{
				this._precision = 6;
			}

			var s:String = t.toString();
			var di:int = s.indexOf('.');
			var l:int = t.length();
			if (0 == this._precision)
			{
				t._delete(di, int.MAX_VALUE); //TODO:
			}
			else if (l > di + this._precision)
			{
				t._delete(di + this._precision + 1, int.MAX_VALUE); //TODO:
			}
			else
			{
				for(; l <= di + this._precision; ++l)
				{
					t.append('0'.charCodeAt());
				}
			}
		}
		
		private function zeroPad(t:StringBuffer):void
		{
			if (this._zero && t.length() < this._width)
			{
				var at:int = t.charAt(0) == '-'.charCodeAt() ? 1 : 0;
				while (t.length() < this._width)
				{
					t.insert(at, '0'.charCodeAt());
				}
			}
		}	
	}
}