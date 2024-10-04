#include <StdInc.h>

#include <random>

#include "ResourceEventComponent.h"

#include "ResourceManagerInstance.h"

fx::TestResourceManager* fx::ResourceManagerInstance::Create()
{
	return new TestResourceManager();
}
