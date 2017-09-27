package com.iteye.weimingtom.metamorphose.launcher;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import com.iteye.weimingtom.metamorphose.lua.BaseLib;
import com.iteye.weimingtom.metamorphose.lua.IOLib;
import com.iteye.weimingtom.metamorphose.lua.Lua;
import com.iteye.weimingtom.metamorphose.lua.LuaTable;
import com.iteye.weimingtom.metamorphose.lua.MathLib;
import com.iteye.weimingtom.metamorphose.lua.OSLib;
import com.iteye.weimingtom.metamorphose.lua.PackageLib;
import com.iteye.weimingtom.metamorphose.lua.StringLib;
import com.iteye.weimingtom.metamorphose.lua.TableLib;

public class Runner {

	public Runner(String[] args, String filename) {
		if (args.length > 0) {
			String content = "";
			FileInputStream fin = null;
			InputStreamReader reader = null;
			BufferedReader buff = null;
			try {
				fin = new FileInputStream(args[0]);
				reader = new InputStreamReader(fin);
				buff = new BufferedReader(reader);
				String line;
				while ((line = buff.readLine()) != null) {
					line = line.trim();
					if (line != null && line.length() > 0) {
						content += line + "\n";
					}
				}
			} catch (IOException e) {
				
			} finally {
				if (buff != null) {
					try {
						buff.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				if (reader != null) {
					try {
						reader.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				if (fin != null) {
					try {
						fin.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			
			final boolean isLoadLib = true;
			final boolean useArg = true;
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
					int narg = args.length;
					LuaTable tbl = L.createTable(narg, narg);
					for (int i = 0; i < narg; i++) {
						L.rawSetI(tbl, i, args[i]);
					}
					L.setGlobal("arg", tbl);
				}
				
				
				int status = L.doString(content);
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
		} else {
			System.out.println("usage: " + filename + " <filename>");
		}
	}
	
	public static void main(String[] args) {
		new Runner(new String[]{"fib.lua"}, "Runner");
	}

}
