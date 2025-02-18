/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "Group.h"
#include "GroupHandler.h"
#include "Game/GlobalUnsynced.h"
#include "Sim/Units/UnitHandler.h"
#include "System/EventHandler.h"
#include "System/creg/STL_Set.h"
#include "System/float3.h"

#include "System/Misc/TracyDefs.h"

CR_BIND(CGroup, (0, 0))
CR_REG_METADATA(CGroup, (
	CR_MEMBER(id),
	CR_MEMBER(ghIndex),
	CR_MEMBER(units),

	CR_POSTLOAD(PostLoad)
))


void CGroup::PostLoad()
{
	RECOIL_DETAILED_TRACY_ZONE;
	while (!units.empty()) {
		CUnit* unit = unitHandler.GetUnit(*units.begin());

		units.erase(unit->id);
		uiGroupHandlers[ghIndex].SetUnitGroup(unit->id, nullptr);
	}
}

bool CGroup::AddUnit(CUnit* unit)
{
	RECOIL_DETAILED_TRACY_ZONE;
	if (unit->team != ghIndex || unit->noGroup)
		return false;

	units.insert(unit->id);
	uiGroupHandlers[ghIndex].PushGroupChange(id);
	return true;
}

void CGroup::RemoveUnit(CUnit* unit)
{
	RECOIL_DETAILED_TRACY_ZONE;
	if (unit->team != ghIndex)
		return;

	units.erase(unit->id);
	uiGroupHandlers[ghIndex].PushGroupChange(id);
}

void CGroup::RemoveIfEmptySpecialGroup()
{
	RECOIL_DETAILED_TRACY_ZONE;
	if (!units.empty())
		return;

	if (id < CGroupHandler::FIRST_SPECIAL_GROUP)
		return;

	//HACK so Global AI groups do not get erased DEPRECATED
	if (uiGroupHandlers[ghIndex].GetTeam() != gu->myTeam)
		return;

	uiGroupHandlers[ghIndex].RemoveGroup(this);
}

void CGroup::ClearUnits()
{
	RECOIL_DETAILED_TRACY_ZONE;
	assert(!uiGroupHandlers.empty());

	while (!units.empty()) {
		CUnit* unit = unitHandler.GetUnit(*units.begin());
		unit->SetGroup(nullptr);
	}

	uiGroupHandlers[ghIndex].PushGroupChange(id);
}

float3 CGroup::CalculateCenter() const
{
	RECOIL_DETAILED_TRACY_ZONE;
	float3 center;

	if (!units.empty()) {
		for (const int unitID: units) {
			center += (unitHandler.GetUnit(unitID))->pos;
		}
		center /= units.size();
	}

	return center;
}
