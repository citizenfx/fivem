/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "IFood.h"
#include "IBait.h"
#include <om/OMComponent.h>

class Food : public fx::OMClass<Food, IFood>
{
public:
	NS_DECL_IFOOD;
};

result_t Food::Eat(int32_t eaty)
{
	trace("eating %d food\n", eaty);

	return FX_S_OK;
}

result_t Food::SetSubFood(IFood *food)
{
	return FX_S_OK;
}

class BigFood : public fx::OMClass<BigFood, IFood, IBait>
{
private:
	int32_t m_eat;

public:
	NS_DECL_IFOOD;

	NS_DECL_IBAIT;
};

result_t BigFood::Eat(int32_t eaty)
{
	trace("eating %d big food\n", eaty * 2);

	m_eat = eaty;

	return FX_S_OK;
}

result_t BigFood::SetSubFood(IFood *food)
{
	return FX_S_OK;
}

result_t BigFood::Bait(int32_t baity)
{
	trace("baity bait %d (with our eat %d)\n", baity, m_eat);

	return FX_S_OK;
}

static const guid_t CLSID_Food =
{ 0x1d9a55af, 0xbb3e, 0x46be,{ 0x86, 0x9, 0xf0, 0xab, 0x4c, 0x4f, 0x61, 0x67 } };

static const guid_t CLSID_BigFood =
{ 0x1d9a55af, 0xbb3e, 0x46bd,{ 0x86, 0x9, 0xf0, 0xab, 0x4c, 0x4f, 0x61, 0x67 } };

static OMFactoryDefinition foodFactoryClsid(CLSID_Food, [] ()
{
	fxIBase* food = new Food();
	food->AddRef();

	return food;
});

static OMFactoryDefinition foodFactoryClsidBig(CLSID_BigFood, [] ()
{
	fxIBase* food = static_cast<IFood*>(new BigFood());
	food->AddRef();

	return food;
});

static OMImplements foodImplements(CLSID_Food, IFood::GetIID());
static OMImplements foodImplementsBig(CLSID_BigFood, IFood::GetIID());
static OMImplements foodImplementsBig2(CLSID_BigFood, IBait::GetIID());

static InitFunction initFunction([] ()
{
	IFood* ppv1;
	result_t hr = fxCreateObjectInstance(CLSID_Food, IFood::GetIID(), (void**)&ppv1);

	ppv1->Eat(50);

	guid_t clsid;
	intptr_t findHandle = fxFindFirstImpl(IFood::GetIID(), &clsid);

	if (findHandle != 0)
	{
		do 
		{
			IFood* ppv2;

			hr = fxCreateObjectInstance(clsid, IFood::GetIID(), (void**)&ppv2);

			ppv2->Eat(69);

			IBait* ppvBait;
			if (ppv2->QueryInterface(IBait::GetIID(), (void**)&ppvBait) == 0)
			{
				ppvBait->Bait(123);

				ppvBait->Release();
			}

			ppv2->Release();
		} while (fxFindNextImpl(findHandle, &clsid));

		fxFindImplClose(findHandle);
	}

	__debugbreak();

	IFood* food = new Food();
	food->AddRef();
	
	IFood* ppv = nullptr;
	result_t res = food->QueryInterface(IFood::GetIID(), (void**)&ppv);

	__debugbreak();
});