package com.iteye.weimingtom.metamorphose.test;

import com.iteye.weimingtom.metamorphose.lua.BaseLib;
import com.iteye.weimingtom.metamorphose.lua.Lua;
import com.iteye.weimingtom.metamorphose.lua.MathLib;
import com.iteye.weimingtom.metamorphose.lua.OSLib;
import com.iteye.weimingtom.metamorphose.lua.PackageLib;
import com.iteye.weimingtom.metamorphose.lua.StringLib;
import com.iteye.weimingtom.metamorphose.lua.TableLib;

/**
 * 这个类用于测试简单的doString
 */
public class Test001 {
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		final boolean isLoadLib = false;
		try {
			Lua L = new Lua();
			if (isLoadLib) {
				BaseLib.open(L);
				PackageLib.open(L);
				MathLib.open(L);
				OSLib.open(L);
				StringLib.open(L);
				TableLib.open(L);
			}
			int status = L.doString(
				"n = 99 + (1 * 10) / 2 - 0.5;\n" +
				"if n > 10 then return 'Oh, 真的比10还大哦:'..n end\n" +
				"return n\n");
			if (status != 0)
			{
				throw new Exception("Error compiling : " + L.value(1));
			}
			System.out.println("Result >>> " + L.value(1));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
