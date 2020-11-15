#pragma once

#include <atArray.h>
#include <boost/type_index.hpp>

#include <EntitySystem.h>

class CPickup : public fwEntity
{

};

class CObject : public fwEntity
{

};

class CPed : public fwEntity
{

};

struct Vector
{
	float x; // +0
	float y; // +4
	float z; // +8
	float w; // +16
};
