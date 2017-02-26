package com.iteye.weimingtom.metamorphose.java
{
	//注意：这个类不应该由Hashtable以外的类创建
	public class HashtableEnum implements Enumeration
	{
		private var _arr:Array;
		private var _idx:int;
		private var _len:int;
		
		public function HashtableEnum()
		{
			
		}
		
		public function hasMoreElements():Boolean
		{
			return this._idx < this._len;
		}
		
		public function nextElement():Object
		{
			return this._arr[this._idx++];
		}
		
		//注意：仅暴露给Hashtable使用的方法
		public function set arr(arr:Array):void
		{
			if(arr != null)
			{
				this._arr = arr;
				this._idx = 0;
				this._len = this._arr.length;
			}
		}
	}
}