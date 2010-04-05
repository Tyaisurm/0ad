/* Copyright (C) 2010 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_UNITANIMATION
#define INCLUDED_UNITANIMATION

#include "ps/CStr.h"

class CUnit;

/**
 * Deals with synchronisation issues between raw animation data (CModel, CSkeletonAnim)
 * and the simulation system (via CUnit), providing a simple fire-and-forget API to play animations.
 * (This is really just a component of CUnit and could probably be merged back into that class.)
 */
class CUnitAnimation
{
	NONCOPYABLE(CUnitAnimation);
public:
	/**
	 * Construct for a given unit, defaulting to the "idle" animation.
	 */
	CUnitAnimation(CUnit& unit);

	/**
	 * Start playing an animation.
	 * The unit's actor defines the available animations, and if more than one is available
	 * then one is picked at random (with a new random choice each loop).
	 * By default, animations start immediately and run at the given speed with no syncing.
	 * Use SetAnimationSync after this to force a specific timing, if it needs to match the
	 * simulation timing.
	 * Alternatively, set @p desync to a non-zero value (e.g. 0.05) to slightly randomise the
	 * offset and speed, so units don't all move in lockstep.
	 * @param name animation's name ("idle", "walk", etc)
	 * @param once if true then the animation freezes on its last frame; otherwise it loops
	 * @param speed fraction of actor-defined speed to play back at (should typically be 1.0)
	 * @param desync maximum fraction of length/speed to randomly adjust timings (or 0.0 for no desyncing)
	 * @param keepSelection if false then the random actor variation will have the selection @p name added
	 * @param actionSound sound group name to be played at the 'action' point in the animation, or empty string
	 */
	void SetAnimationState(const CStr& name, bool once, float speed, float desync, bool keepSelection, const CStrW& actionSound);

	/**
	 * Adjust the timing of the current animation, so that Update(actionTime) will advance it
	 * to the 'action' point defined in the actor, and so that Update(repeatTime) will do a
	 * complete animation loop.
	 * @param actionTime time between now and when the action should occur, in msec
	 * @param repeatTime time for complete loop of animation, in msec
	 */
	void SetAnimationSync(float actionTime, float repeatTime);

	/**
	 * Advance the animation state.
	 * @param time advance time in msec
	 */
	void Update(float time);

private:
	CUnit& m_Unit;
	CStr m_State;
	bool m_Looping;
	float m_OriginalSpeed;
	float m_Speed;
	float m_Desync;
	CStrW m_ActionSound;
};

#endif // INCLUDED_UNITANIMATION
