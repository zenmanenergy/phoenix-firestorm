/**
 * @file fslocaladjust.cpp
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

#include "llviewerprecompiledheaders.h"

#include "fslocaladjust.h"

#include "llbutton.h"
#include "llfloaterreg.h"
#include "llselectmgr.h"
#include "llspinctrl.h"
#include "lltextbox.h"
#include "lltoolpie.h"
#include "llviewerjointattachment.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llvoavatar.h"
#include "llvoavatarself.h"

namespace
{
    constexpr F32 POS_EPSILON = 0.0005f; // meters
    constexpr F32 ROT_EPSILON = 0.0001f;

    bool same_rotation(const LLQuaternion& a, const LLQuaternion& b)
    {
        // q and -q are the same rotation
        const F32 d = llabs(a.mQ[VX] * b.mQ[VX] + a.mQ[VY] * b.mQ[VY] + a.mQ[VZ] * b.mQ[VZ] + a.mQ[VW] * b.mQ[VW]);
        return d > 1.f - ROT_EPSILON;
    }

    LLQuaternion rotation_from_degrees(const LLVector3& deg)
    {
        LLQuaternion q;
        q.setEulerAngles(deg.mV[VX] * DEG_TO_RAD, deg.mV[VY] * DEG_TO_RAD, deg.mV[VZ] * DEG_TO_RAD);
        return q;
    }
}

//-----------------------------------------------------------------------------
// FSLocalAdjust
//-----------------------------------------------------------------------------

// static
LLViewerObject* FSLocalAdjust::getAdjustableRoot(LLViewerObject* objectp)
{
    if (!objectp || objectp->isDead() || !objectp->isAttachment())
    {
        return nullptr;
    }
    LLViewerObject* root = objectp->getRootEdit();
    if (!root || root->isDead() || !root->isAttachment())
    {
        return nullptr;
    }
    LLVOAvatar* avatar = root->getAvatar();
    if (!avatar || avatar->isSelf())
    {
        return nullptr; // your own attachments can be edited normally
    }
    return root;
}

const FSLocalAdjust::Adjustment* FSLocalAdjust::getAdjustment(const LLUUID& id) const
{
    auto it = mAdjustments.find(id);
    return it != mAdjustments.end() ? &it->second : nullptr;
}

void FSLocalAdjust::onObjectUpdated(LLViewerObject* objectp)
{
    if (mAdjustments.empty() || !objectp)
    {
        return;
    }
    auto it = mAdjustments.find(objectp->getID());
    if (it == mAdjustments.end() || !getAdjustableRoot(objectp) || getAdjustableRoot(objectp) != objectp)
    {
        return;
    }
    apply(objectp, it->second);
}

void FSLocalAdjust::apply(LLViewerObject* objectp, Adjustment& adj)
{
    const LLVector3 cur_pos = objectp->getPosition();
    const LLQuaternion cur_rot = objectp->getRotation();

    // If the object isn't where we last put it, the server (or the wearer) moved it:
    // that's the new real position, and our adjustment goes on top of it.
    if (!adj.mHasBase
        || dist_vec(cur_pos, adj.mAppliedPos) > POS_EPSILON
        || !same_rotation(cur_rot, adj.mAppliedRot))
    {
        adj.mBasePos = cur_pos;
        adj.mBaseRot = cur_rot;
        adj.mHasBase = true;
    }

    adj.mAppliedPos = adj.mBasePos + adj.mOffset;
    adj.mAppliedRot = rotation_from_degrees(adj.mRotationDeg) * adj.mBaseRot;

    objectp->setPosition(adj.mAppliedPos);
    objectp->setRotation(adj.mAppliedRot);
}

void FSLocalAdjust::setAdjustment(const LLUUID& id, const LLVector3& offset, const LLVector3& rotation_deg)
{
    Adjustment& adj = mAdjustments[id];
    adj.mOffset = offset;
    adj.mRotationDeg = rotation_deg;

    if (LLViewerObject* objectp = gObjectList.findObject(id); objectp && getAdjustableRoot(objectp) == objectp)
    {
        apply(objectp, adj);
    }
}

void FSLocalAdjust::clearAdjustment(const LLUUID& id)
{
    auto it = mAdjustments.find(id);
    if (it == mAdjustments.end())
    {
        return;
    }

    // Put the object back where the server says it is
    if (LLViewerObject* objectp = gObjectList.findObject(id); objectp && it->second.mHasBase && getAdjustableRoot(objectp) == objectp)
    {
        objectp->setPosition(it->second.mBasePos);
        objectp->setRotation(it->second.mBaseRot);
    }
    mAdjustments.erase(it);
}

//-----------------------------------------------------------------------------
// FSFloaterLocalAdjust
//-----------------------------------------------------------------------------

FSFloaterLocalAdjust::FSFloaterLocalAdjust(const LLSD& key)
    : LLFloater(key)
{
}

bool FSFloaterLocalAdjust::postBuild()
{
    mTitleText = getChild<LLTextBox>("object_text");
    mStatusText = getChild<LLTextBox>("status_text");

    static const char* pos_names[3] = { "pos_x", "pos_y", "pos_z" };
    static const char* rot_names[3] = { "rot_x", "rot_y", "rot_z" };
    for (int i = 0; i < 3; ++i)
    {
        mPos[i] = getChild<LLSpinCtrl>(pos_names[i]);
        mRot[i] = getChild<LLSpinCtrl>(rot_names[i]);
        mPos[i]->setCommitCallback(boost::bind(&FSFloaterLocalAdjust::onValueChanged, this));
        mRot[i]->setCommitCallback(boost::bind(&FSFloaterLocalAdjust::onValueChanged, this));
    }
    getChild<LLButton>("reset_btn")->setCommitCallback(boost::bind(&FSFloaterLocalAdjust::onReset, this));
    getChild<LLButton>("close_btn")->setCommitCallback(boost::bind(&FSFloaterLocalAdjust::closeFloater, this, false));
    return true;
}

void FSFloaterLocalAdjust::onOpen(const LLSD& key)
{
    mObjectID = key.asUUID();

    std::string title = getString("unknown_object");
    if (LLViewerObject* objectp = FSLocalAdjust::getAdjustableRoot(gObjectList.findObject(mObjectID)))
    {
        LLVOAvatar* avatar = objectp->getAvatar();
        LLStringUtil::format_map_t args;
        args["[AVATAR]"] = avatar ? avatar->getFullname() : std::string("?");
        LLViewerJointAttachment* point = avatar ? avatar->getTargetAttachmentPoint(objectp) : nullptr;
        args["[POINT]"] = point ? point->getName() : std::string("?");
        title = getString("object_label", args);
    }
    mTitleText->setText(title);
    refreshFromStore();
}

void FSFloaterLocalAdjust::refreshFromStore()
{
    const FSLocalAdjust::Adjustment* adj = FSLocalAdjust::getInstance()->getAdjustment(mObjectID);
    for (int i = 0; i < 3; ++i)
    {
        mPos[i]->setValue(adj ? adj->mOffset.mV[i] : 0.f);
        mRot[i]->setValue(adj ? adj->mRotationDeg.mV[i] : 0.f);
    }
}

void FSFloaterLocalAdjust::onValueChanged()
{
    if (!FSLocalAdjust::getAdjustableRoot(gObjectList.findObject(mObjectID)))
    {
        return;
    }
    LLVector3 offset((F32)mPos[0]->getValueF32(), (F32)mPos[1]->getValueF32(), (F32)mPos[2]->getValueF32());
    LLVector3 rot((F32)mRot[0]->getValueF32(), (F32)mRot[1]->getValueF32(), (F32)mRot[2]->getValueF32());
    if (offset.isExactlyZero() && rot.isExactlyZero())
    {
        FSLocalAdjust::getInstance()->clearAdjustment(mObjectID);
    }
    else
    {
        FSLocalAdjust::getInstance()->setAdjustment(mObjectID, offset, rot);
    }
}

void FSFloaterLocalAdjust::onReset()
{
    FSLocalAdjust::getInstance()->clearAdjustment(mObjectID);
    refreshFromStore();
}

void FSFloaterLocalAdjust::draw()
{
    // Grey out the controls while the attachment isn't in view (out of range, taken off...)
    const bool available = FSLocalAdjust::getAdjustableRoot(gObjectList.findObject(mObjectID)) != nullptr;
    for (int i = 0; i < 3; ++i)
    {
        mPos[i]->setEnabled(available);
        mRot[i]->setEnabled(available);
    }
    mStatusText->setVisible(!available);
    LLFloater::draw();
}

// static
void FSFloaterLocalAdjust::showForSelection()
{
    LLViewerObject* objectp = FSLocalAdjust::getAdjustableRoot(LLToolPie::getInstance()->getPick().getObject().get());
    if (!objectp)
    {
        objectp = FSLocalAdjust::getAdjustableRoot(LLSelectMgr::getInstance()->getSelection()->getPrimaryObject());
    }
    if (objectp)
    {
        // One window per attachment, so several can be adjusted side by side
        LLFloaterReg::showInstance("fs_local_adjust", LLSD(objectp->getID()), true);
    }
}
