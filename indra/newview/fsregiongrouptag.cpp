/**
 * @file fsregiongrouptag.cpp
 * @brief Automatically wear a group tag in regions that require it
 *
 * $LicenseInfo:firstyear=2026&license=fsviewerlgpl$
 * Phoenix Firestorm Viewer Source Code
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "fsregiongrouptag.h"

#include "llagent.h"
#include "llgroupactions.h"
#include "llcallbacklist.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"

namespace
{
    constexpr char SETTING_NAME[] = "FSRegionGroupTags";
    constexpr F32  CHECK_INTERVAL = 1.f; // seconds between region checks
}

FSRegionGroupTag::FSRegionGroupTag()
{
    gIdleCallbacks.addFunction(onIdle, this);
}

FSRegionGroupTag::~FSRegionGroupTag()
{
    gIdleCallbacks.deleteFunction(onIdle, this);
}

// static
std::string FSRegionGroupTag::getCurrentRegionName()
{
    LLViewerRegion* region = gAgent.getRegion();
    return region ? region->getName() : std::string();
}

LLUUID FSRegionGroupTag::getGroupForCurrentRegion() const
{
    const std::string region_name = getCurrentRegionName();
    if (region_name.empty() || !gSavedPerAccountSettings.controlExists(SETTING_NAME))
    {
        return LLUUID::null;
    }
    const LLSD map = gSavedPerAccountSettings.getLLSD(SETTING_NAME);
    return map.has(region_name) ? map[region_name].asUUID() : LLUUID::null;
}

void FSRegionGroupTag::setGroupForCurrentRegion(const LLUUID& group_id, bool use_here)
{
    const std::string region_name = getCurrentRegionName();
    if (region_name.empty() || group_id.isNull() || !gSavedPerAccountSettings.controlExists(SETTING_NAME))
    {
        return;
    }

    LLSD map = gSavedPerAccountSettings.getLLSD(SETTING_NAME);
    if (!map.isMap())
    {
        map = LLSD::emptyMap();
    }

    if (use_here)
    {
        // One group per region: this replaces any other group set for it
        map[region_name] = group_id;
        LL_INFOS("RegionGroupTag") << "Region '" << region_name << "' now uses group " << group_id << LL_ENDL;
    }
    else if (map.has(region_name) && map[region_name].asUUID() == group_id)
    {
        map.erase(region_name);
        LL_INFOS("RegionGroupTag") << "Region '" << region_name << "' no longer uses a group" << LL_ENDL;
    }
    gSavedPerAccountSettings.setLLSD(SETTING_NAME, map);

    // Act on it straight away for the region we are standing in
    applyForRegion(region_name);
}

// static
void FSRegionGroupTag::onIdle(void* userdata)
{
    FSRegionGroupTag* self = static_cast<FSRegionGroupTag*>(userdata);
    if (self && self->mCheckTimer.getElapsedTimeF32() >= CHECK_INTERVAL)
    {
        self->mCheckTimer.reset();
        self->checkRegion();
    }
}

void FSRegionGroupTag::checkRegion()
{
    static LLCachedControl<bool> enabled(gSavedSettings, "FSRegionGroupTagsEnabled", true);
    if (!enabled)
    {
        return;
    }

    LLViewerRegion* region = gAgent.getRegion();
    // The region name arrives a moment after we enter it, so wait until it is known
    if (!region || region->getName().empty() || region->getRegionID().isNull())
    {
        return;
    }
    if (region->getRegionID() == mLastRegionID)
    {
        return;
    }
    mLastRegionID = region->getRegionID();
    applyForRegion(region->getName());
}

void FSRegionGroupTag::applyForRegion(const std::string& region_name)
{
    LLUUID group_id;
    if (gSavedPerAccountSettings.controlExists(SETTING_NAME))
    {
        const LLSD map = gSavedPerAccountSettings.getLLSD(SETTING_NAME);
        if (map.has(region_name))
        {
            group_id = map[region_name].asUUID();
        }
    }

    // Ignore groups we are no longer a member of
    if (group_id.notNull() && !gAgent.isInGroup(group_id))
    {
        LL_INFOS("RegionGroupTag") << "Not a member of the group set for '" << region_name << "' any more, ignoring it" << LL_ENDL;
        group_id.setNull();
    }

    if (group_id.notNull())
    {
        if (gAgent.getGroupID() != group_id)
        {
            LL_INFOS("RegionGroupTag") << "Entering '" << region_name << "', activating group " << group_id << LL_ENDL;
            LLGroupActions::activate(group_id);
        }
        mAutoActivatedGroup = group_id;
    }
    else if (mAutoActivatedGroup.notNull())
    {
        // Left a region that needed a tag. Take the tag off, unless the user has already
        // switched to a different group themselves.
        if (gAgent.getGroupID() == mAutoActivatedGroup)
        {
            LL_INFOS("RegionGroupTag") << "Left the region for group " << mAutoActivatedGroup << ", setting active group to None" << LL_ENDL;
            LLGroupActions::activate(LLUUID::null);
        }
        mAutoActivatedGroup.setNull();
    }
}
