package com.iteye.weimingtom.metamorphose.java 
{
	//注意：Character.toString用String.fromCharCode()代替
	public class Character
	{
		public function Character() 
		{
			
		}
		
		public static function isUpperCase(ch:uint):Boolean
		{
			return ch >= 'A'.charCodeAt() && ch <= 'Z'.charCodeAt();
		}
		
		public static function isLowerCase(ch:uint):Boolean
		{
			return ch >= 'a'.charCodeAt() && ch <= 'z'.charCodeAt();
		}
		
		public static function isDigit(ch:uint):Boolean
		{
			return ch >= '0'.charCodeAt() && ch <= '9'.charCodeAt();
		}
		
		public static function toLowerCase(ch:uint):String
		{
			return String.fromCharCode(ch).toLowerCase();	
		}
	}
}