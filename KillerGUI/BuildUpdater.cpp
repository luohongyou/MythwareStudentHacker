#include <iostream>
#include <fstream>
#include <Windows.h>
using namespace std; 
int main()
{
	char strModule[256];
	GetModuleFileName(NULL, strModule, 256);
	std::string a;
	a.assign(strModule);
	a.append("\\..\\");
	SetCurrentDirectory(a.c_str());
	
	string temp; 
	int build;
	ifstream fin("Build.h");
	fin >> temp >> temp >> build;
	//输入："//" "NowBuild" XXXX
	fin.close();
	ofstream fout("Build.h");
	fout<<"// NowBuild "<<build+1<<endl;
	fout<<"// Build.h : 定义程序Build信息"<<endl;
	fout<<endl;
	fout<<"// 版权声明："<<endl;
	fout<<"// Copyright (C) 2013-2022 ETO.                              All Rights Reserved."<<endl;
	fout<<"// Copyright (C) 2018-2022 FreeSTD Inc.                      All Rights Reserved."<<endl;
	fout<<"// Copyright (C) 2018-2022 Chengdu Zuosi Co.,Ltd.            All Rights Reserved."<<endl;
	fout<<"// Copyright (C) 2020-2022 Coral Studio.                     All Rights Reserved."<<endl;
	fout<<"// Copyright (C) 2022      North A516 Treaty Organization.   All Rights Reserved."<<endl;
	fout<<"// Copyright (C) 2022      Sunny Orange Electronics Co.,Ltd. All Rights Reserved."<<endl;
	fout<<endl;
	fout<<"#ifndef _KILLERBUILD_"<<endl;
	fout<<"#define _KILLERBUILD_"<<endl;
	fout<<endl;
	fout<<"#define PROGRAM_BUILD "<<build+1<<endl;
	fout<<"#define PROGRAM_BUILD_S \""<<build+1<<"\""<<endl;
	fout<<endl;
	fout<<"#endif"<<endl;
	fout.close();
	return 0;
}
