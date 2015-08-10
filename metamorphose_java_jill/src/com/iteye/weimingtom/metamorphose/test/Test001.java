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
		final String test001 = "n = 99 + (1 * 10) / 2 - 0.5;\n" +
				"if n > 10 then return 'Oh, 真的比10还大哦:'..n end\n" +
				"return n\n";
		final String test002 = "return _VERSION";
		final String test003 = "return nil";
		
		final boolean isLoadLib = true;
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
			int status = L.doString(test003);
			if (status != 0) {
				Object errObj = L.value(1);
			    Object tostring = L.getGlobal("tostring");
			    L.push(tostring);
			    L.push(errObj);
			    L.call(1, 1);
			    String errObjStr = L.toString(L.value(-1));
				throw new Exception("Error compiling : " + errObjStr);
			} else {
				Object result = L.value(1);
			    Object tostring_ = L.getGlobal("tostring");
			    L.push(tostring_);
			    L.push(result);
			    L.call(1, 1);
			    String resultStr = L.toString(L.value(-1));
				System.out.println("Result >>> " + resultStr);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
