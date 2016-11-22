// WUA_API_DEMO.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <wuapi.h>
#include <iostream>
#include <wuerror.h>
#include <string>
#include <locale>  


using namespace std;


std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

    std::string dblstr(len, '\0');
    len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
                                pstr, wslen /* not necessary NULL-terminated */,
                                &dblstr[0], len,
                                NULL, NULL /* no default char */);

    return dblstr;
}

BSTR ConvertMBSToBSTR(const std::string& str)
{
    int wslen = ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
                                      str.data(), str.length(),
                                      NULL, 0);

    BSTR wsdata = ::SysAllocStringLen(NULL, wslen);
    ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
                          str.data(), str.length(),
                          wsdata, wslen);
    return wsdata;
}


int _tmain(int argc, _TCHAR* argv[])
{
	setlocale( LC_ALL, "chs");
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	IUpdateSession* iUpdate;
	IUpdateSearcher* searcher;
	ISearchResult* results;
	//BSTR criteria = SysAllocString(L"IsInstalled=1 or IsHidden=1 or IsPresent=1");
	//BSTR criteria = SysAllocString(L"IsHidden=1'");
	BSTR criteria = SysAllocString(L"IsInstalled=0 and Type='Software'"); // 线上未安装更新
	//BSTR criteria = SysAllocString(L"IsInstalled=1 or IsPresent=1"); // 本地软件

	hr = CoCreateInstance(CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&iUpdate);
	hr = iUpdate->CreateUpdateSearcher(&searcher);

	wcout << L"Searching for updates ..."<<endl;
	hr = searcher->Search(criteria, &results); 
	SysFreeString(criteria);

	switch(hr)
	{
	case S_OK:
		wcout<<L"List of applicable items on the machine:"<<endl;
		break;
	case WU_E_LEGACYSERVER:
		wcout<<L"No server selection enabled"<<endl;
		wcin.get();
		return 0;
	case WU_E_INVALID_CRITERIA:
		wcout<<L"Invalid search criteria"<<endl;
		wcin.get();
		return 0;
	}

	IUpdateCollection *updateList;
	IUpdate *updateItem;
	LONG updateSize;
	BSTR updateName;

	results->get_Updates(&updateList);
	updateList->get_Count(&updateSize);

	wcout << _T("wo数量:") << updateSize << endl;

	if (updateSize == 0)
	{
		wcout << L"No updates found"<<endl;
	}

	for (LONG i = 0; i < updateSize; i++)
	{
		IStringCollection *KBCollection;
		LONG KBCount;
		updateList->get_Item(i,&updateItem);
		updateItem->get_Title(&updateName);

		std::wstring ws(updateName, SysStringLen(updateName));
		//cout<<i+1<<" - "<<updateName<<endl;
		wcout<<i+1<<" - "<<ws<<endl;
		wcout<<i<<endl;

		//IUpdateCollection *updtCollection;
		//LONG updtBundledCount;        
		////Retrieve the bundled updates
		//updateItem->get_BundledUpdates(&updtCollection);
		//hr = updtCollection->get_Count(&updtBundledCount);
		//if ((updtBundledCount>0) && (hr ==S_OK))
		//{
		//	wcout << L"Bundled Updates " <<(updtBundledCount) << endl;
		//	for(LONG j=0;j<updtBundledCount;j++)
		//	{
		//		IUpdate *bundledUpdateItem;
		//		BSTR bundledName;
		//		updtCollection->get_Item(j,&bundledUpdateItem);   
		//		bundledUpdateItem->get_Title(&bundledName);
		//		wcout<<L"    "<<j+1<<" - "<<bundledName<<endl;
		//	}
		//}
	}
	::CoUninitialize();
	wcin.get();
	wcout<<L"end prog"<<endl;
	return 0;
}



//#include "stdafx.h"
//#include <wuapi.h>
//#include <iostream>
//#include <wuerror.h>
//
//using namespace std;
//
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//
//	return 0;
//}