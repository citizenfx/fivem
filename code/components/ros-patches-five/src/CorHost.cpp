/****************************** Module Header ******************************\
* Module Name:  RuntimeHostV4.cpp
* Project:      CppHostCLR
* Copyright (c) Microsoft Corporation.
*
* The code in this file demonstrates using .NET Framework 4.0 Hosting
* Interfaces (http://msdn.microsoft.com/en-us/library/dd380851.aspx) to host
* .NET runtime 4.0, load a .NET assebmly, and invoke a type in the assembly.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "StdInc.h"

#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

// Import mscorlib.tlb (Microsoft Common Language Runtime Class Library).
#include <mscorlib.tlh>
using namespace mscorlib;

#include <atlsafe.h>

//
//   FUNCTION: RuntimeHostV4Demo1(PCWSTR, PCWSTR)
//
//   PURPOSE: The function demonstrates using .NET Framework 4.0 Hosting 
//   Interfaces to host a .NET runtime, and use the ICorRuntimeHost interface
//   that was provided in .NET v1.x to load a .NET assembly and invoke its 
//   type. 
//   
//   If the .NET runtime specified by the pszVersion parameter cannot be 
//   loaded into the current process, the function prints ".NET runtime <the 
//   runtime version> cannot be loaded", and return.
//   
//   If the .NET runtime is successfully loaded, the function loads the 
//   assembly identified by the pszAssemblyName parameter. Next, the function 
//   instantiates the class (pszClassName) in the assembly, calls its 
//   ToString() member method, and print the result. Last, the demo invokes 
//   the public static function 'int GetStringLength(string str)' of the class 
//   and print the result too.
//
//   PARAMETERS:
//   * pszVersion - The desired DOTNETFX version, in the format ¡°vX.X.XXXXX¡±. 
//     The parameter must not be NULL. It¡¯s important to note that this 
//     parameter should match exactly the directory names for each version of
//     the framework, under C:\Windows\Microsoft.NET\Framework[64]. The 
//     current possible values are "v1.0.3705", "v1.1.4322", "v2.0.50727" and 
//     "v4.0.30319". Also, note that the ¡°v¡± prefix is mandatory.
//   * pszAssemblyName - The display name of the assembly to be loaded, such 
//     as "CSClassLibrary". The ".DLL" file extension is not appended.
//   * pszClassName - The name of the Type that defines the method to invoke.
//
//   RETURN VALUE: HRESULT of the demo.
//
HRESULT RunCor(PCWSTR pszVersion, PCWSTR pszAssemblyName,
    PCWSTR pszClassName, std::string* outArg)
{
    HRESULT hr;

    ICLRMetaHost *pMetaHost = NULL;
    ICLRRuntimeInfo *pRuntimeInfo = NULL;

    // ICorRuntimeHost and ICLRRuntimeHost are the two CLR hosting interfaces
    // supported by CLR 4.0. Here we demo the ICorRuntimeHost interface that 
    // was provided in .NET v1.x, and is compatible with all .NET Frameworks. 
    ICorRuntimeHost *pCorRuntimeHost = NULL;

    IUnknownPtr spAppDomainThunk = NULL;
    _AppDomainPtr spDefaultAppDomain = NULL;

    // The .NET assembly to load.
    bstr_t bstrAssemblyName(pszAssemblyName);
    _AssemblyPtr spAssembly = NULL;

    // The .NET class to instantiate.
    bstr_t bstrClassName(pszClassName);
    _TypePtr spType = NULL;
    variant_t vtObject;
    variant_t vtEmpty;

    // The static method in the .NET class to invoke.
    bstr_t bstrStaticMethodName(L"Main");
    SAFEARRAY *psaStaticMethodArgs = NULL;
    variant_t vtStringArg(L"HelloWorld");
    variant_t vtLengthRet;

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // 
    // Load and start the .NET runtime.
    // 

    hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&pMetaHost));
    if (FAILED(hr))
    {
        trace("CLRCreateInstance failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Get the ICLRRuntimeInfo corresponding to a particular CLR version. It 
    // supersedes CorBindToRuntimeEx with STARTUP_LOADER_SAFEMODE.
    hr = pMetaHost->GetRuntime(pszVersion, IID_PPV_ARGS(&pRuntimeInfo));
    if (FAILED(hr))
    {
        trace("ICLRMetaHost::GetRuntime failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Check if the specified runtime can be loaded into the process. This 
    // method will take into account other runtimes that may already be 
    // loaded into the process and set pbLoadable to TRUE if this runtime can 
    // be loaded in an in-process side-by-side fashion. 
    BOOL fLoadable;
    hr = pRuntimeInfo->IsLoadable(&fLoadable);
    if (FAILED(hr))
    {
        trace("ICLRRuntimeInfo::IsLoadable failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    if (!fLoadable)
    {
        trace(".NET runtime %s cannot be loaded\n", ToNarrow(pszVersion));
        goto Cleanup;
    }

    // Load the CLR into the current process and return a runtime interface 
    // pointer. ICorRuntimeHost and ICLRRuntimeHost are the two CLR hosting  
    // interfaces supported by CLR 4.0. Here we demo the ICorRuntimeHost 
    // interface that was provided in .NET v1.x, and is compatible with all 
    // .NET Frameworks. 
    hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost,
        IID_PPV_ARGS(&pCorRuntimeHost));
    if (FAILED(hr))
    {
        trace("ICLRRuntimeInfo::GetInterface failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Start the CLR.
    GetLastError();
    hr = pCorRuntimeHost->Start();
    if (FAILED(hr))
    {
        trace("CLR failed to start w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // 
    // Load the NET assembly. Call the static method GetStringLength of the 
    // class CSSimpleObject. Instantiate the class CSSimpleObject and call 
    // its instance method ToString.
    // 

    // The following C++ code does the same thing as this C# code:
    // 
    //   Assembly assembly = AppDomain.CurrentDomain.Load(pszAssemblyName);
    //   object length = type.InvokeMember("GetStringLength", 
    //       BindingFlags.InvokeMethod | BindingFlags.Static | 
    //       BindingFlags.Public, null, null, new object[] { "HelloWorld" });
    //   object obj = assembly.CreateInstance("CSClassLibrary.CSSimpleObject");
    //   object str = type.InvokeMember("ToString", 
    //       BindingFlags.InvokeMethod | BindingFlags.Instance | 
    //       BindingFlags.Public, null, obj, new object[] { });

    // Get a pointer to the default AppDomain in the CLR.
    hr = pCorRuntimeHost->GetDefaultDomain(&spAppDomainThunk);
    if (FAILED(hr))
    {
        trace("ICorRuntimeHost::GetDefaultDomain failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain));
    if (FAILED(hr))
    {
        trace("Failed to get default AppDomain w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

	{
		FILE* f = _wfopen(bstrAssemblyName, L"rb");

		if (!f)
		{
			trace("Failed to open assembly file.\n");
			ExitProcess(0);
		}

		fseek(f, 0, SEEK_END);

		int size = ftell(f);

		fseek(f, 0, SEEK_SET);

		std::vector<uint8_t> data(size);
		fread(&data[0], 1, data.size(), f);

		fclose(f);

		// Load the .NET assembly.
		CComSafeArray<BYTE> safeArray;
		safeArray.Add(size, data.data(), TRUE);


		hr = spDefaultAppDomain->Load_3(*safeArray.GetSafeArrayPtr(), &spAssembly);
	}

    if (FAILED(hr))
    {
        trace("Failed to load the assembly w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Get the Type of CSSimpleObject.
    hr = spAssembly->GetType_2(bstrClassName, &spType);
    if (FAILED(hr))
    {
        trace("Failed to get the Type interface w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Call the static method of the class: 
    //   public static int GetStringLength(string str);

    // Create a safe array to contain the arguments of the method. The safe 
    // array must be created with vt = VT_VARIANT because .NET reflection 
    // expects an array of Object - VT_VARIANT. There is only one argument, 
    // so cElements = 1.
    psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);
    LONG index = 0;
    hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vtStringArg);
    if (FAILED(hr))
    {
        trace("SafeArrayPutElement failed w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Invoke the "GetStringLength" method from the Type interface.
    hr = spType->InvokeMember_3(bstrStaticMethodName, static_cast<BindingFlags>(
        BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
        NULL, vtEmpty, psaStaticMethodArgs, &vtLengthRet);
    if (FAILED(hr))
    {
        trace("Failed to invoke GetStringLength w/hr 0x%08lx\n", hr);
        goto Cleanup;
    }

    *outArg = ToNarrow(std::wstring(vtLengthRet.bstrVal, SysStringLen(vtLengthRet.bstrVal)));

Cleanup:

    if (pMetaHost)
    {
        pMetaHost->Release();
        pMetaHost = NULL;
    }
    if (pRuntimeInfo)
    {
        pRuntimeInfo->Release();
        pRuntimeInfo = NULL;
    }
    if (pCorRuntimeHost)
    {
        // Please note that after a call to Stop, the CLR cannot be 
        // reinitialized into the same process. This step is usually not 
        // necessary. You can leave the .NET runtime loaded in your process.
        //wprintf(L"Stop the .NET runtime\n");
        //pCorRuntimeHost->Stop();

        pCorRuntimeHost->Release();
        pCorRuntimeHost = NULL;
    }

    if (psaStaticMethodArgs)
    {
        SafeArrayDestroy(psaStaticMethodArgs);
        psaStaticMethodArgs = NULL;
    }

    return hr;
}
