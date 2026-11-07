/**
 * @file fslocaladjust.h
 * @brief Move or rotate attachments other people are wearing, only in your own view
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

#ifndef FS_LOCALADJUST_H
#define FS_LOCALADJUST_H

#include "llfloater.h"
#include "llsingleton.h"
#include "lluuid.h"
#include "llquaternion.h"
#include "v3math.h"

class LLSpinCtrl;
class LLTextBox;
class LLViewerObject;

// <FS:Steve> Local adjustments for attachments worn by other avatars. Nothing is sent to the
// server: the offsets live only in this viewer, and are put back on top of the real position
// every time the server sends an update for the attachment. Kept for the session, by the
// attachment's object ID (which changes when the wearer re-attaches it or relogs).
class FSLocalAdjust : public LLSingleton<FSLocalAdjust>
{
    LLSINGLETON_EMPTY_CTOR(FSLocalAdjust);

public:
    struct Adjustment
    {
        LLVector3    mOffset;      // meters, in the attachment point's frame
        LLVector3    mRotationDeg; // degrees around the attachment's own X, Y and Z

        // Bookkeeping so updates from the server become the new base
        bool         mHasBase = false;
        LLVector3    mBasePos;
        LLQuaternion mBaseRot;
        LLVector3    mAppliedPos;
        LLQuaternion mAppliedRot;
    };

    // Called after every object update; does nothing for objects without an adjustment
    void onObjectUpdated(LLViewerObject* objectp);

    bool hasAdjustment(const LLUUID& id) const { return mAdjustments.count(id) > 0; }
    const Adjustment* getAdjustment(const LLUUID& id) const;
    void setAdjustment(const LLUUID& id, const LLVector3& offset, const LLVector3& rotation_deg);
    void clearAdjustment(const LLUUID& id);

    // The root of another avatar's attachment, or nullptr
    static LLViewerObject* getAdjustableRoot(LLViewerObject* objectp);

private:
    void apply(LLViewerObject* objectp, Adjustment& adj);

    std::map<LLUUID, Adjustment> mAdjustments;
};

class FSFloaterLocalAdjust : public LLFloater
{
public:
    FSFloaterLocalAdjust(const LLSD& key);

    bool postBuild() override;
    void onOpen(const LLSD& key) override;
    void draw() override;

    // Open the floater for the attachment under the last right-click
    static void showForSelection();

private:
    void onValueChanged();
    void onReset();
    void refreshFromStore();

    LLUUID       mObjectID;
    LLTextBox*   mTitleText = nullptr;
    LLTextBox*   mStatusText = nullptr;
    LLSpinCtrl*  mPos[3] = {};
    LLSpinCtrl*  mRot[3] = {};
};
// </FS:Steve>

#endif // FS_LOCALADJUST_H
