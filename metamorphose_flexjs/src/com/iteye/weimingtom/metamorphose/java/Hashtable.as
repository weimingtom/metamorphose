package com.iteye.weimingtom.metamorphose.java
{
	public class Hashtable
	{
		//Dictionary支持用Object作为键，而Array会对键进行toString的转换
		private var _dic:Object = new Object();

		public function Hashtable(initialCapacity:int = 11)
		{
			
		}
		
		public function rehash():void
		{
			
		}
		
		public function keys():Enumeration
		{
			var enum_:HashtableEnum = new HashtableEnum();
			var arr:Array = new Array();
			for(var key:Object in this._dic)
			{
				arr.push(key);
			}
			enum_.arr = arr;
			return enum_;
		}
		
		public function _get(key:Object):Object
		{
			return this._dic[key];
		}
		
		public function put(key:Object, value:Object):Object
		{
			var pre:Object = this._dic[key];
			this._dic[key] = value;
			return pre;
		}
		
		public function remove(key:Object):Object
		{
			var pre:Object = null;
			if(this._dic[key])
			{
				pre = this._dic[key];
				this._dic[key] = null;
				delete this._dic[key];
			}
			return pre;
		}
	}
}