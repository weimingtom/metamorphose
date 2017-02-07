package com.iteye.weimingtom.metamorphose.java 
{
	/**
	 * Stack 类表示后进先出（LIFO）的对象堆栈。
	 * 它通过五个操作对类 Vector 进行了扩展 ，
	 * 允许将向量视为堆栈。
	 * 它提供了通常的 push 和 pop 操作，
	 * 以及取栈顶点的 peek 方法、
	 * 测试堆栈是否为空的 empty 方法、
	 * 在堆栈中查找项并确定到栈顶距离的 search 方法。
	 * 首次创建堆栈时，它不包含数据项。
	 * 
	 * 在Java中Stack继承Vector，需要注意转换问题。
	 */
	public class Stack extends Object
	{
		private var _arr:Array = new Array();
		
		public function Stack() 
		{
			
		}
		
		/**
		 *  相当于push
		 */
		public function addElement(o:Object):Object
		{
			this._arr.push(o);
			return o;
		}
		
		public function lastElement():Object
		{
			var len:int = this._arr.length;
			if(len > 0)
			{
				//trace("lastElement:", this._arr[len - 1]);
				return this._arr[len - 1];
			}
			return null;
		}
		
		public function get size():int
		{
			return this._arr.length;
		}
		
		/**
		 * 设置此向量的大小。
		 * ]如果新大小大于当前大小，则会在向量的末尾添加相应数量的 null 项。
		 * 如果新大小小于当前大小，
		 * 则丢弃索引 newSize 处及其之后的所有项。
		 */
		//TODO:
		public function set size(size:int):void
		{
			var i:int;
			var len:int = this._arr.length;
			if(size >= 0)
			{
				if(size > len)
				{
					for(i = 0; i < size - len; i++)
					{
						//this._arr.push(new Object());
						this._arr.push(null);
					}
				}
				else
				{
					for(i = 0; i < len - size; i++)
					{
						this._arr.pop();
					}
				}
			}
		}
		
		public function pop():Object
		{
			var obj:Object = this._arr.pop();
			return obj;
		}
		
		public function elementAt(i:int):Object
		{
			var obj:Object = this._arr[i];
			return obj;
		}
	}

}