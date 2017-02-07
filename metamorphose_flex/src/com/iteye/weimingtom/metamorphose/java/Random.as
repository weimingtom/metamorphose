package com.iteye.weimingtom.metamorphose.java
{
	public final class Random
	{
		public function Random()
		{
			
		}
		
		public function nextDouble():Number
		{
			return Math.random();
		}
		
		public function nextInt(i:int):int
		{
			return Math.floor(Math.random() * i);
		}
		
		public function setSeed(seed:int):void
		{
			
		}
	}
}