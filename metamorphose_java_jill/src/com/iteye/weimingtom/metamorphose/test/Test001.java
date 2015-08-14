package com.iteye.weimingtom.metamorphose.test;

import com.iteye.weimingtom.metamorphose.lua.BaseLib;
import com.iteye.weimingtom.metamorphose.lua.IOLib;
import com.iteye.weimingtom.metamorphose.lua.Lua;
import com.iteye.weimingtom.metamorphose.lua.LuaTable;
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
		final String test004 = "io.write(\"Hello world, from \",_VERSION,\"!\\n\")";
		final String test005 = "function printf(...)\n"+
			" io.write(string.format(...))\n" + 
			"end\n" +
			"\n" +
			"" +
			//"printf(\"Hello %%s from %%s on %%s\\n\",os.getenv\"USER\" or \"there\",_VERSION,os.date())";
			"printf(\"Hello %s from %s on %s\\n\",os.getenv\"USER\" or \"there\",_VERSION,os.date())";
		final String test006 = "-- echo command line arguments\n" + 
			"\n" +
			"for i=0,table.getn(arg) do\n"+
			" print(i,arg[i])\n"+
			"end";
		final String test007 = "-- read environment variables as if they were global variables\n"+
			"\n"+
			"local f=function (t,i) return os.getenv(i) end\n"+
			"setmetatable(getfenv(),{__index=f})\n"+
			"\n"+
			"-- an example\n"+
			"print(a,USER,PATH)\n";
	
		final String test008 = "-- example of for with generator functions\n"+
			"\n"+
			"function generatefib (n)\n"+
			"  return coroutine.wrap(function ()\n"+
			"    local a,b = 1, 1\n"+
			"    while a <= n do\n"+
			"      coroutine.yield(a)\n"+
			"      a, b = b, a+b\n"+
			"    end\n"+
			"  end)\n"+
			"end\n"+
			"\n"+
			"for i in generatefib(1000) do print(i) end\n";
	
		final boolean isLoadLib = true;
		final boolean useArg = true;
		final String[] argv = new String[]{"hello", "world"};
		try {
			Lua L = new Lua();
			if (isLoadLib) {
				BaseLib.open(L);
				PackageLib.open(L);
				MathLib.open(L);
				OSLib.open(L);
				StringLib.open(L);
				TableLib.open(L);
				IOLib.open(L);
			}
			if (useArg) {
				//FIXME: index may be minus (for example, arg[-1], before script file name)
				//@see http://www.ttlsa.com/lua/lua-install-and-lua-variable-ttlsa/
				int narg = argv.length;
				LuaTable tbl = L.createTable(narg, narg);
				for (int i = 0; i < narg; i++) {
					L.rawSetI(tbl, i, argv[i]);
				}
				L.setGlobal("arg", tbl);
			}
			
			
			int status = L.doString(test008);
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
				System.err.println("Result >>> " + resultStr);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
