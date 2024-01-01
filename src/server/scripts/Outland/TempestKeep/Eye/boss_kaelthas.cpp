/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "Opcodes.h"
#include "ScriptedCreature.h"
#include "SpellScriptLoader.h"
#include "WorldPacket.h"
#include "the_eye.h"

enum Yells
{
    // Kael'thas Speech
    SAY_INTRO                           = 0,
    SAY_INTRO_CAPERNIAN                 = 1,
    SAY_INTRO_TELONICUS                 = 2,
    SAY_INTRO_THALADRED                 = 3,
    SAY_INTRO_SANGUINAR                 = 4,
    SAY_PHASE2_WEAPON                   = 5,
    SAY_PHASE3_ADVANCE                  = 6,
    SAY_PHASE4_INTRO2                   = 7,
    SAY_PHASE5_NUTS                     = 8,
    SAY_SLAY                            = 9,
    SAY_MINDCONTROL                     = 10,
    SAY_GRAVITYLAPSE                    = 11,
    SAY_SUMMON_PHOENIX                  = 12,
    SAY_DEATH                           = 13,
    SAY_PYROBLAST                       = 14,

    // Advisors
    SAY_THALADRED_AGGRO                 = 0,
    SAY_SANGUINAR_AGGRO                 = 0,
    SAY_CAPERNIAN_AGGRO                 = 0,
    SAY_TELONICUS_AGGRO                 = 0
};

enum Spells
{
    // Phase 2 spells
    SPELL_SUMMON_WEAPONS                = 36976,
    SPELL_SUMMON_WEAPONA                = 36958,
    SPELL_SUMMON_WEAPONB                = 36959,
    SPELL_SUMMON_WEAPONC                = 36960,
    SPELL_SUMMON_WEAPOND                = 36961,
    SPELL_SUMMON_WEAPONE                = 36962,
    SPELL_SUMMON_WEAPONF                = 36963,
    SPELL_SUMMON_WEAPONG                = 36964,

    // Phase 3 spells
    SPELL_RESURRECTION                  = 36450,

    // Phase 4 spells
    SPELL_FIREBALL                      = 36805,
    SPELL_ARCANE_DISRUPTION             = 36834,
    SPELL_PHOENIX                       = 36723,
    SPELL_MIND_CONTROL                  = 36797,
    SPELL_SHOCK_BARRIER                 = 36815,
    SPELL_PYROBLAST                     = 36819,
    SPELL_FLAME_STRIKE                  = 36735,
    SPELL_FLAME_STRIKE_DAMAGE           = 36731,

    // Event
    SPELL_NETHERBEAM_AURA1              = 36364,
    SPELL_NETHERBEAM_AURA2              = 36370,
    SPELL_NETHERBEAM_AURA3              = 36371,
    SPELL_NETHERBEAM1                   = 36089,
    SPELL_NETHERBEAM2                   = 36090,
    SPELL_KAEL_GAINING_POWER            = 36091,
    SPELL_KAEL_EXPLODES1                = 36376,
    SPELL_KAEL_EXPLODES2                = 36375,
    SPELL_KAEL_EXPLODES3                = 36373,
    SPELL_KAEL_EXPLODES4                = 36354,
    SPELL_KAEL_EXPLODES5                = 36092,
    SPELL_GROW                          = 36184,
    SPELL_KEAL_STUNNED                  = 36185,
    SPELL_KAEL_FULL_POWER               = 36187,
    SPELL_FLOATING_DROWNED              = 36550,

    SPELL_PURE_NETHER_BEAM1             = 36196,
    SPELL_PURE_NETHER_BEAM2             = 36197,
    SPELL_PURE_NETHER_BEAM3             = 36198,
    SPELL_PURE_NETHER_BEAM4             = 36201,
    SPELL_PURE_NETHER_BEAM5             = 36290,
    SPELL_PURE_NETHER_BEAM6             = 36291,

    SPELL_DARK_BANISHED_STATE           = 52241, //wrong visual apparently - sniffs?
    SPELL_ARCANE_EXPLOSION_VISUAL       = 34807,
    SPELL_KAEL_PHASE_TWO                = 36709,

    // Phase 5 spells
    SPELL_GRAVITY_LAPSE                 = 35941,
    SPELL_GRAVITY_LAPSE_TELEPORT1       = 35966,
    SPELL_GRAVITY_LAPSE_KNOCKBACK       = 34480,
    SPELL_GRAVITY_LAPSE_AURA            = 39432,
    SPELL_SUMMON_NETHER_VAPOR           = 35865,
    SPELL_NETHER_BEAM                   = 35869,
    SPELL_NETHER_BEAM_DAMAGE            = 35873,

    SPELL_REMOTE_TOY_STUN               = 37029
};

enum Groups
{
    GROUP_SHOCK_BARRIER                 = 0
};

enum Misc
{
    POINT_MIDDLE                        = 1,
    POINT_AIR                           = 2,
    POINT_START_LAST_PHASE              = 3,
    DATA_RESURRECT_CAST                 = 1,
    NPC_WORLD_TRIGGER                   = 19871,
    NPC_NETHER_VAPOR                    = 21002,

    PHASE_NONE                          = 0,
    PHASE_SINGLE_ADVISOR                = 1,
    PHASE_WEAPONS                       = 2,
    PHASE_ALL_ADVISORS                  = 3,
    PHASE_FINAL                         = 4,

    EVENT_PREFIGHT_PHASE11              = 1,
    EVENT_PREFIGHT_PHASE12              = 2,
    EVENT_PREFIGHT_PHASE21              = 3,
    EVENT_PREFIGHT_PHASE22              = 4,
    EVENT_PREFIGHT_PHASE31              = 5,
    EVENT_PREFIGHT_PHASE32              = 6,
    EVENT_PREFIGHT_PHASE41              = 7,
    EVENT_PREFIGHT_PHASE42              = 8,
    EVENT_PREFIGHT_PHASE51              = 9,
    EVENT_PREFIGHT_PHASE52              = 10,
    EVENT_PREFIGHT_PHASE61              = 11,
    EVENT_PREFIGHT_PHASE62              = 12,
    EVENT_PREFIGHT_PHASE63              = 13,
    EVENT_PREFIGHT_PHASE71              = 14,
    EVENT_GATHER_ADVISORS               = 15,

    EVENT_SPELL_SEQ_1                   = 30,
    EVENT_SPELL_SEQ_2                   = 31,
    EVENT_SPELL_SEQ_3                   = 32,
    EVENT_SPELL_FIREBALL                = 33,
    EVENT_SPELL_PYROBLAST               = 34,
    EVENT_SPELL_FLAMESTRIKE             = 35,
    EVENT_SPELL_ARCANE_DISRUPTION       = 36,
    EVENT_SPELL_MIND_CONTROL            = 37,
    EVENT_SPELL_SUMMON_PHOENIX          = 38,
    EVENT_CHECK_HEALTH                  = 39,
    EVENT_SPELL_GRAVITY_LAPSE           = 40,
    EVENT_GRAVITY_LAPSE_END             = 41,
    EVENT_SPELL_SHOCK_BARRIER           = 42,
    EVENT_SPELL_NETHER_BEAM             = 43,
    EVENT_SPELL_NETHER_VAPOR            = 44,

    EVENT_SCENE_1                       = 50,
    EVENT_SCENE_2                       = 51,
    EVENT_SCENE_3                       = 52,
    EVENT_SCENE_4                       = 53,
    EVENT_SCENE_5                       = 54,
    EVENT_SCENE_6                       = 55,
    EVENT_SCENE_7                       = 56,
    EVENT_SCENE_8                       = 57,
    EVENT_SCENE_9                       = 58,
    EVENT_SCENE_10                      = 59,
    EVENT_SCENE_11                      = 60,
    EVENT_SCENE_12                      = 61,
    EVENT_SCENE_13                      = 62,
    EVENT_SCENE_14                      = 63,
    EVENT_SCENE_15                      = 64,
    EVENT_SCENE_16                      = 65
};

const Position triggersPos[6] =
{
    {799.11f, -38.95f, 85.0f, 0.0f},
    {800.16f, 37.65f, 85.0f, 0.0f},
    {847.64f, -16.19f, 64.05f, 0.0f},
    {847.53f, 15.01f, 63.69f, 0.0f},
    {843.44f, -7.87f, 67.14f, 0.0f},
    {843.35f, 6.35f, 67.14f, 0.0f}
};

struct boss_kaelthas : public BossAI
{
    boss_kaelthas(Creature* creature) : BossAI(creature, DATA_KAELTHAS)
    {
        scheduler.SetValidator([this]{
            return me->HasUnitState(UNIT_STATE_CASTING);
        });
    }

    uint8 phase;
    EventMap events2;

    void PrepareAdvisors()
    {
        for (uint8 i = DATA_KAEL_ADVISOR1; i <= DATA_KAEL_ADVISOR4; ++i)
            if (Creature* advisor = ObjectAccessor::GetCreature(*me, instance->GetGuidData(i)))
            {
                advisor->Respawn(true);
                advisor->StopMovingOnCurrentPos();
                advisor->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                advisor->SetReactState(REACT_PASSIVE);
                summons.Summon(advisor);
            }
    }

    void SetData(uint32 type, uint32 data) override
    {
        if (type == DATA_RESURRECT_CAST && data == DATA_RESURRECT_CAST)
        {
            for (SummonList::const_iterator i = summons.begin(); i != summons.end(); ++i)
            {
                if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                {
                    if (summon->GetSpawnId())
                    {
                        summon->SetReactState(REACT_PASSIVE);
                        summon->setDeathState(DeathState::JustRespawned);
                        summon->SetUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                    }
                }
            }
        }
    }

    void SetRoomState(GOState state)
    {
        if (GameObject* window = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(GO_BRIDGE_WINDOW)))
            window->SetGoState(state);
        if (GameObject* window = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(GO_KAEL_STATUE_RIGHT)))
            window->SetGoState(state);
        if (GameObject* window = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(GO_KAEL_STATUE_LEFT)))
            window->SetGoState(state);
    }

    void Reset() override
    {
        BossAI::Reset();
        _rpScheduler.CancelAll();
        _rpScheduler.Schedule(1s, [this](TaskContext)
        {
            PrepareAdvisors();
        });

        phase = PHASE_NONE;

        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HOVER, true);
        me->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
        SetRoomState(GO_STATE_READY);
        me->SetDisableGravity(false);
        me->SetWalk(false);

        ScheduleHealthCheckEvent(50, [&]
        {
            scheduler.CancelAll();
            me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MovePoint(POINT_MIDDLE, me->GetHomePosition(), true, true);
            me->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
            me->SendMeleeAttackStop();
        });
    }

    void AttackStart(Unit* who) override
    {
        if (phase == PHASE_FINAL && events.GetNextEventTime(EVENT_GRAVITY_LAPSE_END) == 0)
            BossAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (phase == PHASE_NONE && who->GetTypeId() == TYPEID_PLAYER && me->IsValidAttackTarget(who))
        {
            phase = PHASE_SINGLE_ADVISOR;
            me->SetInCombatWithZone();
            Talk(SAY_INTRO);
            events2.ScheduleEvent(EVENT_PREFIGHT_PHASE11, 23000);
            events2.ScheduleEvent(EVENT_PREFIGHT_PHASE12, 30000);
            ScheduleUniqueTimedEvent(23s, [&]
            {
                Talk(SAY_INTRO_THALADRED);
            }, EVENT_PREFIGHT_PHASE11);
            ScheduleUniqueTimedEvent(30s, [&]
            {
                if (Creature* advisor = summons.GetCreatureWithEntry(NPC_THALADRED))
                {
                    advisor->SetReactState(REACT_AGGRESSIVE);
                    advisor->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                    {
                        advisor->AI()->AttackStart(target);
                    }
                    advisor->SetInCombatWithZone();
                    advisor->AI()->Talk(SAY_THALADRED_AGGRO);
                }
            }, EVENT_PREFIGHT_PHASE12);
        }
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            Talk(SAY_SLAY);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        if (summon->GetEntry() == NPC_NETHER_VAPOR)
            summon->GetMotionMaster()->MoveRandom(20.0f);
    }

    void SummonedCreatureDies(Creature* summon, Unit*) override
    {
        if (phase == PHASE_FINAL)
            return;

        if (summon->GetSpawnId() && phase == PHASE_ALL_ADVISORS)
        {
            /*
            for (SummonList::const_iterator i = summons.begin(); i != summons.end(); ++i)
                if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                    if (summon->GetSpawnId() && summon->IsAlive())
                        return;
            */
            summons.DoForAllSummons([&](WorldObject* summon)
            {
                if (summon->ToCreature()->GetSpawnId() && summon->ToCreature()->IsAlive())
                {
                    return;
                }
            });

            ScheduleUniqueTimedEvent(2s, [&]
            {
                Talk(SAY_PHASE4_INTRO2);
                phase = PHASE_FINAL;
                DoResetThreatList();
                me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT FLAG_DISABLE_MOVE);
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                {
                    AttackStart(target);
                }
                scheduler.CancelAll();
                ScheduleTimedEvent(1s, [&]
                {
                    DoCastVictim(SPELL_FIREBALL);
                }, 2000ms, 3200ms);
                ScheduleTimedEvent(15s, [&]
                {
                    DoCastRandomTarget(SPELL_FLAME_STRIKE, 0, 100.0f, true);
                }, 20s);
                ScheduleTimedEvent(30s, [&]
                {
                    Talk(SAY_SUMMON_PHOENIX);
                    DoCastSelf(SPELL_PHOENIX);
                }, 40s);
                ScheduleTimedEvent(20s, [&]
                {
                    if (roll_chance_i(50))
                    {
                        Talk(SAY_MINDCONTROL);
                    }
                    me->CastCustomSpell(SPELL_MIND_CONTROL, SPELLVALUE_MAX_TARGETS, 3, me, false);
                    ScheduleUniqueTimedEvent(3s, [&]
                    {
                        DoCastSelf(SPELL_ARCANE_DISRUPTION);
                    }, EVENT_SPELL_ARCANE_DISRUPTION);
                }, 50s);
                ScheduleTimedEvent(40s, [&]
                {
                    ScheduleUniqueTimedEvent(3s, [&]
                    {
                        if (roll_chance_i(50))
                        {
                            Talk(SAY_MINDCONTROL);
                        }
                        me->CastCustomSpell(SPELL_MIND_CONTROL, SPELLVALUE_MAX_TARGETS, 3, me, false);
                    }, EVENT_SPELL_MIND_CONTROL);
                    ScheduleUniqueTimedEvent(6s, [&]
                    {
                        DoCastSelf(SPELL_ARCANE_DISRUPTION);
                    }, EVENT_SPELL_ARCANE_DISRUPTION);
                }, 50s);
                ScheduleTimedEvent(60s, [&]
                {
                    DoCastSelf(SPELL_SHOCK_BARRIER);
                    scheduler.DelayAll(10s);
                    ScheduleTimedEvent(0s, [&]
                    {
                        DoCastVictim(SPELL_PYROBLAST);
                    }, 4s, EVENT_SPELL_PYROBLAST);
                    ScheduleUniqueTimedEvent(9500ms, [&]
                    {
                        events.CancelEvent(EVENT_SPELL_PYROBLAST);
                    }, EVENT_SPELL_PYROBLAST);
                }, 50s);
            }, EVENT_PREFIGHT_PHASE71);
            return;
        }

        if (summon->GetEntry() == NPC_THALADRED)
        {
            ScheduleUniqueTimedEvent(2s, [&]
            {
                Talk(SAY_INTRO_SANGUINAR);
            }, EVENT_PREFIGHT_PHASE21);
            ScheduleUniqueTimedEvent(14500ms, [&]
            {
                if (Creature* advisor = summons.GetCreatureWithEntry(NPC_LORD_SANGUINAR))
                {
                    advisor->SetReactState(REACT_AGGRESSIVE);
                    advisor->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                    {
                        advisor->AI()->AttackStart(target);
                    }
                    advisor->SetInCombatWithZone();
                    advisor->AI()->Talk(SAY_SANGUINAR_AGGRO);
                }
            }, EVENT_PREFIGHT_PHASE22);
        }
        else if (summon->GetEntry() == NPC_LORD_SANGUINAR)
        {
            ScheduleUniqueTimedEvent(2s, [&]
            {
                Talk(SAY_INTRO_CAPERNIAN);
            }, EVENT_PREFIGHT_PHASE31);
            ScheduleUniqueTimedEvent(9s, [&]
            {
                if (Creature* advisor = summons.GetCreatureWithEntry(NPC_CAPERNIAN))
                {
                    advisor->SetReactState(REACT_AGGRESSIVE);
                    advisor->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                        advisor->AI()->AttackStart(target);
                    advisor->SetInCombatWithZone();
                    advisor->AI()->Talk(SAY_CAPERNIAN_AGGRO);
                }
            }, EVENT_PREFIGHT_PHASE32);
        }
        else if (summon->GetEntry() == NPC_CAPERNIAN)
        {
            ScheduleUniqueTimedEvent(2s, [&]
            {
                Talk(SAY_INTRO_TELONICUS);
            }, EVENT_PREFIGHT_PHASE41);
            ScheduleUniqueTimedEvent(10400ms, [&]
            {
                if (Creature* advisor = summons.GetCreatureWithEntry(NPC_TELONICUS))
                {
                    advisor->SetReactState(REACT_AGGRESSIVE);
                    advisor->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                        advisor->AI()->AttackStart(target);
                    advisor->SetInCombatWithZone();
                    advisor->AI()->Talk(SAY_TELONICUS_AGGRO);
                }
            }, EVENT_PREFIGHT_PHASE42);
        }
        else if (summon->GetEntry() == NPC_TELONICUS)
        {
            events2.ScheduleEvent(EVENT_PREFIGHT_PHASE51, 3000);
            events2.ScheduleEvent(EVENT_PREFIGHT_PHASE52, 9000);
            ScheduleUniqueTimedEvent(3s, [&]
            {
                Talk(SAY_PHASE2_WEAPON);
                me->CastSpell(me, SPELL_SUMMON_WEAPONS, false);
                phase = PHASE_WEAPONS;
            }, EVENT_PREFIGHT_PHASE51);
            ScheduleUniqueTimedEvent(9s, [&]
            {
                for (SummonList::const_iterator i = summons.begin(); i != summons.end(); ++i)
                {
                    if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                        if (!summon->GetSpawnId())
                        {
                            summon->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            summon->SetInCombatWithZone();
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                                summon->AI()->AttackStart(target);
                        }
                }
                events2.ScheduleEvent(EVENT_PREFIGHT_PHASE61, 2 * MINUTE * IN_MILLISECONDS);
                events2.ScheduleEvent(EVENT_PREFIGHT_PHASE62, 2 * MINUTE * IN_MILLISECONDS + 6000);
                events2.ScheduleEvent(EVENT_PREFIGHT_PHASE63, 2 * MINUTE * IN_MILLISECONDS + 12000);
            }, EVENT_PREFIGHT_PHASE52);
        }
    }

    void JustDied(Unit* killer) override
    {
        me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        Talk(SAY_DEATH);
        BossAI::JustDied(killer);
    }

    void MovementInform(uint32 type, uint32 point) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (point == POINT_MIDDLE)
        {
            ScheduleUniqueTimedEvent(0s, [&]
            {
                me->SetTarget();
                me->SetFacingTo(M_PI);
                me->SetWalk(true);
                Talk(SAY_PHASE5_NUTS);
            }, EVENT_SCENE_1);
            ScheduleUniqueTimedEvent(2500ms, [&]
            {
                me->SetTarget();
                DoCastSelf(SPELL_KAEL_EXPLODES1, true);
                DoCastSelf(SPELL_KAEL_GAINING_POWER, true);
                me->SetDisableGravity(true);
            }, EVENT_SCENE_2);
            ScheduleUniqueTimedEvent(4s, [&]
            {
                me->SetTarget();
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (Creature* trigger = me->SummonCreature                      (WORLD_TRIGGER, triggersPos[i], TEMPSUMMON_TIMED_DESPAWN, 60000))
                    {
                        trigger->CastSpell(me, SPELL_NETHERBEAM1 + i, false);
                    }
                }
                me->GetMotionMaster()->MovePoint(POINT_AIR, me->GetPositionX(), me->GetPositionY(), 76.0f, false, true);
                DoCastSelf(SPELL_GROW, true);
            }, EVENT_SCENE_3);
            ScheduleUniqueTimedEvent(7s, [&]
            {
                me->SetTarget();
                DoCastSelf(SPELL_GROW, true);
                DoCastSelf(SPELL_KAEL_EXPLODES2, true);
                DoCastSelf(SPELL_NETHERBEAM_AURA1, true);
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, triggersPos[i + 2], TEMPSUMMON_TIMED_DESPAWN, 60000))
                    {
                        trigger->CastSpell(me, SPELL_NETHERBEAM1 + i, false);
                    }
                }
            }, EVENT_SCENE_4);
            ScheduleUniqueTimedEvent(10s, [&]
            {
                me->SetTarget();
                DoCastSelf(SPELL_GROW, true);
                DoCastSelf(SPELL_KAEL_EXPLODES3, true);
                DoCastSelf(SPELL_NETHERBEAM_AURA2, true);
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, triggersPos[i + 4], TEMPSUMMON_TIMED_DESPAWN, 60000))
                    {
                        trigger->CastSpell(me, SPELL_NETHERBEAM1 + i, false);
                    }
                }
            }, EVENT_SCENE_5);
            ScheduleUniqueTimedEvent(14s, [&]
            {
                DoCastSelf(SPELL_GROW, true);
                DoCastSelf(SPELL_KAEL_EXPLODES4, true);
                DoCastSelf(SPELL_NETHERBEAM_AURA3, true);
            }, EVENT_SCENE_6);
            ScheduleUniqueTimedEvent(17500ms, [&]
            {
                SetRoomState(GO_STATE_ACTIVE);
                me->SetUnitMovementFlags(MOVEMENTFLAG_HOVER | MOVEMENTFLAG_WALKING | MOVEMENTFLAG_DISABLE_GRAVITY);
                me->SendMovementFlagUpdate();
            }, EVENT_SCENE_7);
            ScheduleUniqueTimedEvent(19s, [&]
            {
                summons.DespawnEntry(WORLD_TRIGGER);
                me->RemoveAurasDueToSpell(SPELL_NETHERBEAM_AURA1);
                me->RemoveAurasDueToSpell(SPELL_NETHERBEAM_AURA2);
                me->RemoveAurasDueToSpell(SPELL_NETHERBEAM_AURA3);
                DoCastSelf(SPELL_KAEL_EXPLODES5, true);
                DoCastSelf(SPELL_FLOATING_DROWNED, true);
                //DoCastSelf(SPELL_KEAL_STUNNED, true);
            }, EVENT_SCENE_8);
            ScheduleUniqueTimedEvent(22s, [&]
            {
                DoCastSelf(SPELL_DARK_BANISHED_STATE, true);
                DoCastSelf(SPELL_ARCANE_EXPLOSION_VISUAL, true);
                me->SummonCreature(NPC_WORLD_TRIGGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() + 5, me->GetPositionY(), me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM1, true);
                }
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() - 5, me->GetPositionY(), me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM2, true);
                }
            }, EVENT_SCENE_9);
            ScheduleUniqueTimedEvent(22800ms, [&]
            {
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() - 5, me->GetPositionY() - 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM3, true);
                }
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() + 5, me->GetPositionY() + 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM1, true);
                }
            }, EVENT_SCENE_10);
            ScheduleUniqueTimedEvent(23600ms, [&]
            {
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX(), me->GetPositionY() + 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM2, true);
                }
            }, EVENT_SCENE_11);
            ScheduleUniqueTimedEvent(24500ms, [&]
            {
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX(), me->GetPositionY() - 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM3, true);
                }
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() + 5, me->GetPositionY() - 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM1, true);
                }
            }, EVENT_SCENE_12);
            ScheduleUniqueTimedEvent(24800ms, [&]
            {
                if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX() - 5, me->GetPositionY() + 5, me->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                {
                    trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM2, true);
                }
            }, EVENT_SCENE_13);
            ScheduleUniqueTimedEvent(25300ms, [&]
            {
                //if (Creature* trigger = me->SummonCreature(WORLD_TRIGGER, me->GetPositionX()-5, me->GetPositionY()+5, me->GetPositionZ()+15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                //{
                //  trigger->CastSpell(me, SPELL_PURE_NETHER_BEAM3, true);
                //}
            }, EVENT_SCENE_14);
            ScheduleUniqueTimedEvent(32s, [&]
            {
                me->RemoveAurasDueToSpell(SPELL_FLOATING_DROWNED);
                me->RemoveAurasDueToSpell(SPELL_KEAL_STUNNED);
                DoCastSelf(SPELL_KAEL_FULL_POWER, true);
                DoCastSelf(SPELL_KAEL_PHASE_TWO, true);
                DoCastSelf(SPELL_PURE_NETHER_BEAM4, true);
                DoCastSelf(SPELL_PURE_NETHER_BEAM5, true);
                DoCastSelf(SPELL_PURE_NETHER_BEAM6, true);
                me->SetUnitMovementFlags(MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_WALKING);
                me->SendMovementFlagUpdate();
            }, EVENT_SCENE_15);
            ScheduleUniqueTimedEvent(36s, [&]
            {
                summons.DespawnEntry(WORLD_TRIGGER);
                me->RemoveAurasDueToSpell(SPELL_DARK_BANISHED_STATE);
                me->GetMotionMaster()->MovePoint(POINT_START_LAST_PHASE, me->GetHomePosition(), false, true);
            }, EVENT_SCENE_16);
        }
        else if (point == POINT_START_LAST_PHASE)
        {
            me->SetDisableGravity(false);
            me->SetWalk(false);
            me->RemoveAurasDueToSpell(SPELL_KAEL_FULL_POWER);
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            scheduler.DelayAll(60s);
            ScheduleTimedEvent(0ms, [&]
            {
                DoCastVictim(SPELL_FIREBALL);
            }, 2000ms, 3200ms);
            ScheduleTimedEvent(10s, [&]
            {
                DoCastRandomTarget(SPELL_FLAME_STRIKE, 0, 100.0f, true);
            }, 20s);
            ScheduleTimedEvent(20s, [&]
            {
                Talk(SAY_SUMMON_PHOENIX);
                DoCastSelf(SPELL_PHOENIX);
            }, 40s);
            ScheduleTimedEvent(5s, [&]
            {
                scheduler.DelayAll(30s);
                me->setAttackTimer(BASE_ATTACK, 30000);
                ScheduleUniqueTimedEvent(32s, [&]
                {
                    summons.DespawnEntry(NPC_NETHER_VAPOR);
                    events.CancelEvent(EVENT_SPELL_NETHER_BEAM);
                    me->SetTarget(me->GetVictim()->GetGUID());
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
                }, EVENT_GRAVITY_LAPSE_END);
                ScheduleTimedEvent(10s, [&]
                {
                    DoCastSelf(SPELL_SHOCK_BARRIER);
                }, 10s);

                ScheduleTimedEvent(4s, [&]
                {
                    DoCastSelf(SPELL_NETHER_BEAM);
                }, 4s, EVENT_SPELL_NETHER_BEAM);
                DoCastSelf(SPELL_SUMMON_NETHER_VAPOR);
                DoCastSelf(SPELL_SHOCK_BARRIER);
                DoCastSelf(SPELL_GRAVITY_LAPSE);
                me->SetTarget();
                me->GetMotionMaster()->Clear();
                me->StopMoving();
                Talk(SAY_GRAVITYLAPSE);
            }, 90s);
            if (me->GetVictim())
            {
                me->SetTarget(me->GetVictim()->GetGUID());
                AttackStart(me->GetVictim());
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events2.Update(diff);
        switch (events2.ExecuteEvent())
        {

 
            case EVENT_PREFIGHT_PHASE61:
                phase = PHASE_ALL_ADVISORS;
                Talk(SAY_PHASE3_ADVANCE);
                break;
            case EVENT_PREFIGHT_PHASE62:
                me->CastSpell(me, SPELL_RESURRECTION, false);
                break;
            case EVENT_PREFIGHT_PHASE63:
                for (SummonList::const_iterator i = summons.begin(); i != summons.end(); ++i)
                    if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                        if (summon->GetSpawnId())
                        {
                            summon->SetReactState(REACT_AGGRESSIVE);
                            summon->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                            summon->SetInCombatWithZone();
                            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                                summon->AI()->AttackStart(target);
                        }
                events2.ScheduleEvent(EVENT_PREFIGHT_PHASE71, 3 * MINUTE * IN_MILLISECONDS);
                break;
        }

        if (!events2.Empty())
            return;

        if (!UpdateVictim())
            return;

        events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
            case EVENT_SPELL_SEQ_1:
                events.ScheduleEvent(EVENT_SPELL_MIND_CONTROL, 0);
                events.ScheduleEvent(EVENT_SPELL_ARCANE_DISRUPTION, 3000);
                events.ScheduleEvent(EVENT_SPELL_SEQ_1, 50000);
                break;
            case EVENT_SPELL_SEQ_2:
                events.ScheduleEvent(EVENT_SPELL_MIND_CONTROL, 3000);
                events.ScheduleEvent(EVENT_SPELL_ARCANE_DISRUPTION, 6000);
                events.ScheduleEvent(EVENT_SPELL_SEQ_2, 50000);
                break;
            case EVENT_SPELL_SEQ_3:
                Talk(SAY_PYROBLAST);
                me->CastSpell(me, SPELL_SHOCK_BARRIER, false);
                events.ScheduleEvent(EVENT_SPELL_SEQ_3, 50000);
                events.DelayEvents(10000);
                events.ScheduleEvent(EVENT_SPELL_PYROBLAST, 0);
                events.ScheduleEvent(EVENT_SPELL_PYROBLAST, 4000);
                events.ScheduleEvent(EVENT_SPELL_PYROBLAST, 8000);
                break;
            case EVENT_SPELL_PYROBLAST:
                me->CastSpell(me->GetVictim(), SPELL_PYROBLAST, false);
                break;
            case EVENT_SPELL_ARCANE_DISRUPTION:
                me->CastSpell(me, SPELL_ARCANE_DISRUPTION, false);
                break;
            case EVENT_SPELL_MIND_CONTROL:
                if (roll_chance_i(50))
                    Talk(SAY_MINDCONTROL);
                me->CastCustomSpell(SPELL_MIND_CONTROL, SPELLVALUE_MAX_TARGETS, 3, me, false);
                break;
        }

        DoMeleeAttackIfReady();
    }

    bool CheckEvadeIfOutOfCombatArea() const override
    {
        return me->GetHomePosition().GetExactDist2d(me) > 165.0f || !SelectTargetFromPlayerList(165.0f);
    }
private:
    TaskScheduler _rpScheduler;
};
class spell_kaelthas_kael_phase_two : public SpellScriptLoader
{
public:
    spell_kaelthas_kael_phase_two() : SpellScriptLoader("spell_kaelthas_kael_phase_two") { }

    class spell_kaelthas_kael_phase_two_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_kael_phase_two_SpellScript);

        bool Load() override
        {
            if (GetCaster()->GetTypeId() == TYPEID_UNIT)
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* kael = ObjectAccessor::GetCreature(*GetCaster(), instance->GetGuidData(NPC_KAELTHAS)))
                        kael->AI()->SummonedCreatureDies(GetCaster()->ToCreature(), nullptr);
            return true;
        }

        void Register() override
        {
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_kael_phase_two_SpellScript();
    }
};

class spell_kaelthas_remote_toy : public SpellScriptLoader
{
public:
    spell_kaelthas_remote_toy() : SpellScriptLoader("spell_kaelthas_remote_toy") { }

    class spell_kaelthas_remote_toy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kaelthas_remote_toy_AuraScript);

        void HandlePeriodic(AuraEffect const*  /*aurEff*/)
        {
            PreventDefaultAction();
            if (roll_chance_i(66))
                GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_REMOTE_TOY_STUN, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kaelthas_remote_toy_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_kaelthas_remote_toy_AuraScript();
    }
};

class spell_kaelthas_summon_weapons : public SpellScriptLoader
{
public:
    spell_kaelthas_summon_weapons() : SpellScriptLoader("spell_kaelthas_summon_weapons") { }

    class spell_kaelthas_summon_weapons_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_summon_weapons_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitEffect(effIndex);
            for (uint32 i = SPELL_SUMMON_WEAPONA; i <= SPELL_SUMMON_WEAPONG; ++i)
                GetCaster()->CastSpell(GetCaster(), i, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_kaelthas_summon_weapons_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_summon_weapons_SpellScript();
    }
};

class spell_kaelthas_resurrection : public SpellScriptLoader
{
public:
    spell_kaelthas_resurrection() : SpellScriptLoader("spell_kaelthas_resurrection") { }

    class spell_kaelthas_resurrection_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_resurrection_SpellScript);

        void HandleBeforeCast()
        {
            GetCaster()->GetAI()->SetData(DATA_RESURRECT_CAST, DATA_RESURRECT_CAST);
        }

        void Register() override
        {
            BeforeCast += SpellCastFn(spell_kaelthas_resurrection_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_resurrection_SpellScript();
    }
};

class spell_kaelthas_mind_control : public SpellScriptLoader
{
public:
    spell_kaelthas_mind_control() : SpellScriptLoader("spell_kaelthas_mind_control") { }

    class spell_kaelthas_mind_control_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_mind_control_SpellScript);

        void SelectTarget(std::list<WorldObject*>& targets)
        {
            if (Unit* victim = GetCaster()->GetVictim())
                targets.remove_if(Acore::ObjectGUIDCheck(victim->GetGUID(), true));
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kaelthas_mind_control_SpellScript::SelectTarget, EFFECT_ALL, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_mind_control_SpellScript();
    }
};

class spell_kaelthas_burn : public SpellScriptLoader
{
public:
    spell_kaelthas_burn() : SpellScriptLoader("spell_kaelthas_burn") { }

    class spell_kaelthas_burn_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kaelthas_burn_AuraScript);

        void HandlePeriodic(AuraEffect const*  /*aurEff*/)
        {
            Unit::DealDamage(GetUnitOwner(), GetUnitOwner(), GetUnitOwner()->CountPctFromMaxHealth(5) + 1);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kaelthas_burn_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_kaelthas_burn_AuraScript();
    }
};

class spell_kaelthas_flame_strike : public SpellScriptLoader
{
public:
    spell_kaelthas_flame_strike() : SpellScriptLoader("spell_kaelthas_flame_strike") { }

    class spell_kaelthas_flame_strike_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kaelthas_flame_strike_AuraScript);

        bool Load() override
        {
            return GetUnitOwner()->GetTypeId() == TYPEID_UNIT;
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetUnitOwner()->RemoveAllAuras();
            GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_FLAME_STRIKE_DAMAGE, true);
            GetUnitOwner()->ToCreature()->DespawnOrUnsummon(2000);
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_kaelthas_flame_strike_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_kaelthas_flame_strike_AuraScript();
    }
};

class lapseTeleport : public BasicEvent
{
public:
    lapseTeleport(Player* owner) : _owner(owner)
    {
    }

    bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
    {
        if (_owner->IsBeingTeleportedNear())
            _owner->m_Events.AddEvent(new lapseTeleport(_owner), _owner->m_Events.CalculateTime(1));
        else if (!_owner->IsBeingTeleported())
        {
            _owner->CastSpell(_owner, SPELL_GRAVITY_LAPSE_KNOCKBACK, true);
            _owner->CastSpell(_owner, SPELL_GRAVITY_LAPSE_AURA, true);
        }
        return true;
    }

private:
    Player* _owner;
};

class spell_kaelthas_gravity_lapse : public SpellScriptLoader
{
public:
    spell_kaelthas_gravity_lapse() : SpellScriptLoader("spell_kaelthas_gravity_lapse") { }

    class spell_kaelthas_gravity_lapse_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_gravity_lapse_SpellScript);

        bool Load() override
        {
            _currentSpellId = SPELL_GRAVITY_LAPSE_TELEPORT1;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitEffect(effIndex);
            if (_currentSpellId < SPELL_GRAVITY_LAPSE_TELEPORT1 + 25)
                if (Player* target = GetHitPlayer())
                {
                    GetCaster()->CastSpell(target, _currentSpellId++, true);
                    target->m_Events.AddEvent(new lapseTeleport(target), target->m_Events.CalculateTime(1));
                }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_kaelthas_gravity_lapse_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }

    private:
        uint32 _currentSpellId;
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_gravity_lapse_SpellScript();
    }
};

class spell_kaelthas_nether_beam : public SpellScriptLoader
{
public:
    spell_kaelthas_nether_beam() : SpellScriptLoader("spell_kaelthas_nether_beam") { }

    class spell_kaelthas_nether_beam_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_nether_beam_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitEffect(effIndex);

            ThreatContainer::StorageType const& ThreatList = GetCaster()-> GetThreatMgr().GetThreatList();
            std::list<Unit*> targetList;
            for (ThreatContainer::StorageType::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Unit* target = ObjectAccessor::GetUnit(*GetCaster(), (*itr)->getUnitGuid());
                if (target && target->GetTypeId() == TYPEID_PLAYER)
                    targetList.push_back(target);
            }

            Acore::Containers::RandomResize(targetList, 5);
            for (std::list<Unit*>::const_iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                GetCaster()->CastSpell(*itr, SPELL_NETHER_BEAM_DAMAGE, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_kaelthas_nether_beam_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_nether_beam_SpellScript();
    }
};

class spell_kaelthas_summon_nether_vapor : public SpellScriptLoader
{
public:
    spell_kaelthas_summon_nether_vapor() : SpellScriptLoader("spell_kaelthas_summon_nether_vapor") { }

    class spell_kaelthas_summon_nether_vapor_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kaelthas_summon_nether_vapor_SpellScript);

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            PreventHitEffect(effIndex);
            for (uint32 i = 0; i < 5; ++i)
                GetCaster()->SummonCreature(NPC_NETHER_VAPOR, GetCaster()->GetPositionX() + 6 * cos(i / 5.0f * 2 * M_PI), GetCaster()->GetPositionY() + 6 * std::sin(i / 5.0f * 2 * M_PI), GetCaster()->GetPositionZ() + 7.0f + i, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_kaelthas_summon_nether_vapor_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kaelthas_summon_nether_vapor_SpellScript();
    }
};

void AddSC_boss_kaelthas()
{
    new boss_kaelthas();
    new spell_kaelthas_kael_phase_two();
    new spell_kaelthas_remote_toy();
    new spell_kaelthas_summon_weapons();
    new spell_kaelthas_resurrection();
    new spell_kaelthas_mind_control();
    new spell_kaelthas_burn();
    new spell_kaelthas_flame_strike();
    new spell_kaelthas_gravity_lapse();
    new spell_kaelthas_nether_beam();
    new spell_kaelthas_summon_nether_vapor();
}

