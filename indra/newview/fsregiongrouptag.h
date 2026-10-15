/**
 * @file fsregiongrouptag.h
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

#ifndef FS_REGIONGROUPTAG_H
#define FS_REGIONGROUPTAG_H

#include "llsingleton.h"
#include "lluuid.h"
#include "llframetimer.h"

// <FS:Steve> Some regions only let you in (or only let you use them) while a particular
// group is your active group. This remembers which group goes with which region: arriving
// in such a region makes that group active, and leaving it for a region without a group
// sets your active group back to None, so you are not still wearing the tag elsewhere.
//
// The list is per account and is edited from the group profile ("Wear this group's tag in
// this region"). It is stored in the per-account setting FSRegionGroupTags as a map of
// region name -> group ID.
class FSRegionGroupTag : public LLSingleton<FSRegionGroupTag>
{
    LLSINGLETON(FSRegionGroupTag);
    ~FSRegionGroupTag();

public:
    // Name of the region we are in, or empty if not known yet
    static std::string getCurrentRegionName();

    // The group set for the current region, or null
    LLUUID getGroupForCurrentRegion() const;

    // Set or clear the group for the current region. Setting it also makes it active now.
    void setGroupForCurrentRegion(const LLUUID& group_id, bool use_here);

private:
    static void onIdle(void* userdata);
    void checkRegion();
    void applyForRegion(const std::string& region_name);

    LLUUID       mLastRegionID;
    LLUUID       mAutoActivatedGroup; // group we made active for the current region
    LLFrameTimer mCheckTimer;
};
// </FS:Steve>

#endif // FS_REGIONGROUPTAG_H
