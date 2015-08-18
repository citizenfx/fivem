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

FX_DEFINE_GUID(CLSID_Food, 0x1d9a55af, 0xbb3e, 0x46be, 0x86, 0x9, 0xf0, 0xab, 0x4c, 0x4f, 0x61, 0x67);
FX_DEFINE_GUID(CLSID_BigFood, 0x1d9a55af, 0xbb3e, 0x46bd, 0x86, 0x9, 0xf0, 0xab, 0x4c, 0x4f, 0x61, 0x67);

FX_NEW_FACTORY(Food);
FX_NEW_FACTORY(BigFood);

FX_IMPLEMENTS(CLSID_Food, IFood);
FX_IMPLEMENTS(CLSID_BigFood, IFood);
FX_IMPLEMENTS(CLSID_BigFood, IBait);

static InitFunction initFunction([] ()
{
	{
		fx::OMPtr<IFood> ppv1;
		fx::MakeInterface(&ppv1, CLSID_Food);

		ppv1->Eat(50);
	}

	guid_t clsid;
	intptr_t findHandle = fxFindFirstImpl(IFood::GetIID(), &clsid);

	if (findHandle != 0)
	{
		do 
		{
			fx::OMPtr<IFood> ppv2;
			fx::MakeInterface(&ppv2, clsid);

			ppv2->Eat(69);

			{
				fx::OMPtr<IBait> ppvBait;
				if (FX_SUCCEEDED(ppv2.As(&ppvBait)))
				{
					ppvBait->Bait(123);
				}
			}
		} while (fxFindNextImpl(findHandle, &clsid));

		fxFindImplClose(findHandle);
	}

	//__debugbreak();
});