package com.iteye.weimingtom.metamorphose.java 
{
	public class IllegalArgumentException extends Error
	{
		public function IllegalArgumentException(str:String = "") 
		{
			this.message = str;
		}	
	}
}