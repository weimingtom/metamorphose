package com.iteye.weimingtom.metamorphose.test  
{
	import com.iteye.weimingtom.metamorphose.lua.BaseLib;
	import com.iteye.weimingtom.metamorphose.lua.IOLib;
	import com.iteye.weimingtom.metamorphose.lua.Lua;
	import com.iteye.weimingtom.metamorphose.lua.MathLib;
	import com.iteye.weimingtom.metamorphose.lua.OSLib;
	import com.iteye.weimingtom.metamorphose.lua.PackageLib;
	import com.iteye.weimingtom.metamorphose.lua.StringLib;
	import com.iteye.weimingtom.metamorphose.lua.TableLib;
	
	import flash.display.MovieClip;
	
	/**
	 * 这个类用于测试简单的doString
	 */
	public final class Test001 extends MovieClip
	{		
		public function Test001() 
		{
			const test001:String = "n = 99 + (1 * 10) / 2 - 0.5;\n" +
				"if n > 10 then return 'Oh, 真的比10还大哦:'..n end\n" +
				"return n\n";
			const test002:String = "return _VERSION"
			const test003:String = "return nil";
			
			const isLoadLib:Boolean = true;
			try
			{
				trace("Start test...");
				var L:Lua = new Lua();
				if(isLoadLib)
				{
					BaseLib.open(L);
					PackageLib.open(L);
					MathLib.open(L);
					OSLib.open(L);
					StringLib.open(L);
					TableLib.open(L);
					IOLib.open(L);
				}
				var status:int = L.doString(test003);
				if (status != 0)
				{
					var errObj:Object = L.value(1);
					var tostring:Object = L.getGlobal("tostring");
					L.pushObject(tostring);
					L.pushObject(errObj);
					L.call(1, 1);
					var errObjStr:String = L.toString(L.value(-1));
					throw new Error("Error compiling : " + L.value(1));
				} 
				else 
				{
					var result:Object = L.value(1);
					var tostring_:Object = L.getGlobal("tostring");
					L.pushObject(tostring_);
					L.pushObject(result);
					L.call(1, 1);
					var resultStr:String = L.toString(L.value(-1));
					trace("Result >>> " + resultStr);
				}
			}
			catch (e:Error)
			{
				trace(e.getStackTrace());
			}
		}
	}
}

