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

/* ScriptData
SDName: Bosses_Opera
SD%Complete: 90
SDComment: Oz, Hood, and RAJ event implemented. RAJ event requires more testing.
SDCategory: Karazhan
EndScriptData */

#include "Player.h"
#include "ScriptMgr.h"
#include "TaskScheduler.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "karazhan.h"

/***********************************/
/*** OPERA WIZARD OF OZ EVENT *****/
/*********************************/
enum Says
{
    SAY_DOROTHEE_DEATH          = 0,
    SAY_DOROTHEE_SUMMON         = 1,
    SAY_DOROTHEE_TITO_DEATH     = 2,
    SAY_DOROTHEE_AGGRO          = 3,

    SAY_ROAR_AGGRO              = 0,
    SAY_ROAR_DEATH              = 1,
    SAY_ROAR_SLAY               = 2,

    SAY_STRAWMAN_AGGRO          = 0,
    SAY_STRAWMAN_DEATH          = 1,
    SAY_STRAWMAN_SLAY           = 2,

    SAY_TINHEAD_AGGRO           = 0,
    SAY_TINHEAD_DEATH           = 1,
    SAY_TINHEAD_SLAY            = 2,
    EMOTE_RUST                  = 3,

    SAY_CRONE_AGGRO             = 0,
    SAY_CRONE_DEATH             = 1,
    SAY_CRONE_SLAY              = 2,
};

enum Spells
{
    // Dorothee
    SPELL_WATERBOLT         = 31012,
    SPELL_SCREAM            = 31013,
    SPELL_SUMMONTITO        = 31014,

    // Tito
    SPELL_YIPPING           = 31015,

    // Strawman
    SPELL_BRAIN_BASH        = 31046,
    SPELL_BRAIN_WIPE        = 31069,
    SPELL_BURNING_STRAW     = 31075,

    // Tinhead
    SPELL_CLEAVE            = 31043,
    SPELL_RUST              = 31086,

    // Roar
    SPELL_MANGLE            = 31041,
    SPELL_SHRED             = 31042,
    SPELL_FRIGHTENED_SCREAM = 31013,

    // Crone
    SPELL_CHAIN_LIGHTNING   = 32337,

    // Cyclone
    SPELL_KNOCKBACK         = 32334,
    SPELL_CYCLONE_VISUAL    = 32332,
};

enum Creatures
{
    CREATURE_TITO           = 17548,
    CREATURE_CYCLONE        = 18412,
    CREATURE_CRONE          = 18168,
};

void SummonCroneIfReady(InstanceScript* instance, Creature* creature)
{
    instance->SetData(DATA_OPERA_OZ_DEATHCOUNT, SPECIAL);  // Increment DeathCount

    if (instance->GetData(DATA_OPERA_OZ_DEATHCOUNT) == 4)
    {
        if (Creature* pCrone = creature->SummonCreature(CREATURE_CRONE, -10891.96f, -1755.95f, creature->GetPositionZ(), 4.64f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, HOUR * 2 * IN_MILLISECONDS))
        {
            if (creature->GetVictim())
                pCrone->AI()->AttackStart(creature->GetVictim());
            pCrone->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            pCrone->SetImmuneToPC(false);
        }
    }
}

struct boss_dorothee : public ScriptedAI
{
    boss_dorothee(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
        //this is kinda a big no-no. but it will prevent her from moving to chase targets. she should just cast her spells. in this case, since there is not really something to LOS her with or get out of range this would work. but a more elegant solution would be better
        Initialize();
        instance = creature->GetInstanceScript();
    }



    void Initialize()
    {
        WaterBoltTimer = 0;
        FearTimer = 15000;
        SummonTitoTimer = 41000;

        TitoDied = false;
        _introDone = false;
    }

    InstanceScript* instance;
    bool TitoDied;

    uint32 AggroTimer;

    uint32 WaterBoltTimer;
    uint32 FearTimer;
    uint32 SummonTitoTimer;


    void Reset() override
    {
        Initialize();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->SetInCombatWithZone();

        _scheduler.Schedule(1ms, [this](TaskContext context)
        {
            DoCastRandomTarget(SPELL_WATERBOLT);
            context.Repeat(1500ms);
        }).Schedule(15s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SCREAM);
            context.Repeat(30s);
        }).Schedule(41s, [this](TaskContext)
        {
            SummonTito();
        });
    }

    void JustReachedHome() override
    {
        me->DespawnOrUnsummon();
    }

    void SummonTito();

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DOROTHEE_DEATH);
        SummonCroneIfReady(instance, me);
        me->DespawnOrUnsummon();
    }

    void AttackStart(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode(EvadeReason reason) override
    {
        ScriptedAI::EnterEvadeMode(reason);

        if(!me->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
            me->DespawnOrUnsummon();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if(!_introDone)
        {
            if(!me->IsInEvadeMode())
            {
                Talk(SAY_DOROTHEE_AGGRO);
                _introDone = true;
                _scheduler.Schedule(12s, [this](TaskContext)
                {
                    me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                    me->SetImmuneToPC(false);
                    me->SetInCombatWithZone();
                });
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();

        _scheduler.Update(diff);
    }
private:
    TaskScheduler _scheduler;

    bool _introDone;
};

struct npc_tito : public ScriptedAI
{
    npc_tito(Creature* creature) : ScriptedAI(creature) { }



    ObjectGuid DorotheeGUID;
    uint32 YipTimer;

    void Reset() override
    {
        DorotheeGUID.Clear();
        YipTimer = 10000;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        DoZoneInCombat();

        _scheduler.Schedule(10s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_YIPPING);
            context.Repeat(10s);
        });
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (DorotheeGUID)
        {
            Creature* Dorothee = ObjectAccessor::GetCreature(*me, DorotheeGUID);
            if (Dorothee && Dorothee->IsAlive())
            {
                Talk(SAY_DOROTHEE_TITO_DEATH, Dorothee);
            }
        }
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
        
        DoMeleeAttackIfReady();
    }
private:
    TaskScheduler _scheduler;
};


void boss_dorothee::SummonTito()
{
    if (Creature* pTito = me->SummonCreature(CREATURE_TITO, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000))
    {
        Talk(SAY_DOROTHEE_SUMMON);
        CAST_AI(npc_tito, pTito->AI())->DorotheeGUID = me->GetGUID();
        pTito->AI()->AttackStart(me->GetVictim());
        TitoDied = false;
    }
}

struct boss_roar : public ScriptedAI
{
    boss_roar(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        _scheduler.Schedule(16670ms, [this](TaskContext)
        {
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetImmuneToPC(false);
            me->SetInCombatWithZone();
        });
    }

    void MoveInLineOfSight(Unit* who) override

    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode(EvadeReason reason) override
    {
        ScriptedAI::EnterEvadeMode(reason);

        if(!me->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
            me->DespawnOrUnsummon();
        }
    }

    void AttackStart(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_ROAR_AGGRO);
        DoZoneInCombat();

        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_MANGLE);
            context.Repeat(5s, 8s);
        }).Schedule(10s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SHRED);
            context.Repeat(10s, 15s);
        }).Schedule(15s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_FRIGHTENED_SCREAM);
            context.Repeat(20s, 30s);
        });
    }

    void JustReachedHome() override
    {
        me->DespawnOrUnsummon();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_ROAR_DEATH);
        SummonCroneIfReady(instance, me);
        me->DespawnOrUnsummon();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_ROAR_SLAY);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
private:
    TaskScheduler _scheduler;
};


struct boss_strawman : public ScriptedAI
{
    boss_strawman(Creature* creature) : ScriptedAI(creature)
    { 
        instance = creature->GetInstanceScript();
    }


    InstanceScript* instance;

    void Reset() override
    {
        _scheduler.Schedule(26300ms, [this](TaskContext)
        {
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetImmuneToPC(false);
            me->SetInCombatWithZone();
        });
    }

    void AttackStart(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode(EvadeReason reason) override
    {
        ScriptedAI::EnterEvadeMode(reason);

        if(!me->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
            me->DespawnOrUnsummon();
        }
    }
    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_STRAWMAN_AGGRO);
        DoZoneInCombat();

        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_BRAIN_BASH);
            context.Repeat(15s);
        }).Schedule(7s, [this](TaskContext context)
        {
            DoCastRandomTarget(SPELL_BRAIN_WIPE);
            context.Repeat(20s);
        });
    }

    void JustReachedHome() override
    {
        me->DespawnOrUnsummon();
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* Spell) override
    {
        if ((Spell->SchoolMask == SPELL_SCHOOL_MASK_FIRE) && (!(rand() % 10)))
        {
            /*
                if (not direct damage(aoe, dot))
                    return;
            */

            DoCast(me, SPELL_BURNING_STRAW, true);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_STRAWMAN_DEATH);
        SummonCroneIfReady(instance, me);
        me->DespawnOrUnsummon();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_STRAWMAN_SLAY);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);

        DoMeleeAttackIfReady();
    }
private:
    TaskScheduler _scheduler;
};


struct boss_tinhead : public ScriptedAI
{
    boss_tinhead(Creature* creature) : ScriptedAI(creature) 
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    uint32 AggroTimer;
    uint32 CleaveTimer;
    uint32 RustTimer;

    uint8 RustCount;

    void Reset() override
    {
        _rustCount = 0;

        _scheduler.Schedule(34470ms, [this](TaskContext)
        {
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetImmuneToPC(false);
            me->SetInCombatWithZone();
        });
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_TINHEAD_AGGRO);
        DoZoneInCombat();

        _scheduler.Schedule(5s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_CLEAVE);
            CleaveTimer = 5000;
        }).Schedule(15s, [this](TaskContext context)
        {
            if (_rustCount < 8)
            {
                ++_rustCount;
                Talk(EMOTE_RUST);
                DoCastSelf(SPELL_RUST);
                context.Repeat(6s);
            }
        });
    }

    void JustReachedHome() override
    {
        me->DespawnOrUnsummon();
    }

    void AttackStart(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterEvadeMode(EvadeReason reason) override
    {
        ScriptedAI::EnterEvadeMode(reason);

        if(!me->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
            me->DespawnOrUnsummon();
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_TINHEAD_DEATH);
        SummonCroneIfReady(instance, me);
        me->DespawnOrUnsummon();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_TINHEAD_SLAY);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
private:
    TaskScheduler _scheduler;
    uint8 _rustCount;
};


struct boss_crone : public ScriptedAI
{
    boss_crone(Creature* creature) : CreatureScript(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
    }

    void JustReachedHome() override
    {
        me->DespawnOrUnsummon();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_CRONE_SLAY);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_CRONE_AGGRO);
        DoZoneInCombat();

        _scheduler.Schedule(22s, [this](TaskContext context)
        {
            if (Creature* Cyclone = DoSpawnCreature(CREATURE_CYCLONE, float(urand(0, 9)), float(urand(0, 9)), 0, 0, TEMPSUMMON_TIMED_DESPAWN, 15000))
            Cyclone->CastSpell(Cyclone, SPELL_CYCLONE_VISUAL, true);
            context.Repeat(22s);
        }).Schedule(8s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_CHAIN_LIGHTNING);
            context.Repeat(8s);
        });

    }

    void EnterEvadeMode(EvadeReason reason) override
    {
        ScriptedAI::EnterEvadeMode(reason);

        instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_CRONE_DEATH);

        instance->SetBossState(DATA_OPERA_PERFORMANCE, DONE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);

        DoMeleeAttackIfReady();
    }
private:
    TaskScheduler _scheduler;
};

struct npc_cyclone : public ScriptedAI
{
    npc_cyclone(Creature* creature) : ScriptedAI(creature) { }

    uint32 MoveTimer;

    void Reset() override
    {
        MoveTimer = 1000;
    }

    void JustEngagedWith(Unit* /*who*/) override
    { 
        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            Position pos = me->GetRandomNearPosition(10);
            me->GetMotionMaster()->MovePoint(0, pos);
            context.Repeat(3s, 5s);
        });
    }

    void MoveInLineOfSight(Unit* /*who*/) override

    {
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->HasAura(SPELL_KNOCKBACK))
            DoCast(me, SPELL_KNOCKBACK, true);

        _scheduler.Update(diff);
    }
private:
    TaskScheduler _scheduler;
};


/**************************************/
/**** Opera Red Riding Hood Event* ***/
/************************************/
enum RedRidingHood
{
    SAY_WOLF_AGGRO                  = 0,
    SAY_WOLF_SLAY                   = 1,
    SAY_WOLF_HOOD                   = 2,
    SOUND_WOLF_DEATH                = 9275,

    SPELL_LITTLE_RED_RIDING_HOOD    = 30768,
    SPELL_TERRIFYING_HOWL           = 30752,
    SPELL_WIDE_SWIPE                = 30761,

    CREATURE_BIG_BAD_WOLF           = 17521,
};

#define GOSSIP_GRANDMA          "What phat lewtz you have grandmother?"

class npc_grandmother : public CreatureScript
{
public:
    npc_grandmother() : CreatureScript("npc_grandmother") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            if (Creature* pBigBadWolf = creature->SummonCreature(CREATURE_BIG_BAD_WOLF, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, HOUR * 2 * IN_MILLISECONDS))
                pBigBadWolf->AI()->AttackStart(player);

            creature->DespawnOrUnsummon();
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_GRANDMA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 8990, creature->GetGUID());

        return true;
    }
};

class boss_bigbadwolf : public CreatureScript
{
public:
    boss_bigbadwolf() : CreatureScript("boss_bigbadwolf") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetKarazhanAI<boss_bigbadwolfAI>(creature);
    }

    struct boss_bigbadwolfAI : public ScriptedAI
    {
        boss_bigbadwolfAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 ChaseTimer;
        uint32 FearTimer;
        uint32 SwipeTimer;

        ObjectGuid HoodGUID;
        float TempThreat;

        bool IsChasing;

        void Reset() override
        {
            ChaseTimer = 30000;
            FearTimer = urand(25000, 35000);
            SwipeTimer = 5000;

            HoodGUID.Clear();
            TempThreat = 0;

            IsChasing = false;
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            Talk(SAY_WOLF_AGGRO);
            DoZoneInCombat();
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_WOLF_SLAY);
        }

        void JustReachedHome() override
        {
            me->DespawnOrUnsummon();
        }

        void EnterEvadeMode(EvadeReason reason) override
        {
            ScriptedAI::EnterEvadeMode(reason);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
        }

        void JustDied(Unit* /*killer*/) override
        {
            DoPlaySoundToSet(me, SOUND_WOLF_DEATH);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, DONE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();

            if (ChaseTimer <= diff)
            {
                if (!IsChasing)
                {
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                    {
                        Talk(SAY_WOLF_HOOD);
                        DoCast(target, SPELL_LITTLE_RED_RIDING_HOOD, true);
                        TempThreat = DoGetThreat(target);
                        if (TempThreat)
                            DoModifyThreatByPercent(target, -100);
                        HoodGUID = target->GetGUID();
                        me->AddThreat(target, 1000000.0f);
                        ChaseTimer = 20000;
                        IsChasing = true;
                    }
                }
                else
                {
                    IsChasing = false;

                    if (Unit* target = ObjectAccessor::GetUnit(*me, HoodGUID))
                    {
                        HoodGUID.Clear();
                        if (DoGetThreat(target))
                            DoModifyThreatByPercent(target, -100);
                        me->AddThreat(target, TempThreat);
                        TempThreat = 0;
                    }

                    ChaseTimer = 40000;
                }
            }
            else
                ChaseTimer -= diff;

            if (IsChasing)
                return;

            if (FearTimer <= diff)
            {
                DoCastVictim(SPELL_TERRIFYING_HOWL);
                FearTimer = urand(25000, 35000);
            }
            else
                FearTimer -= diff;

            if (SwipeTimer <= diff)
            {
                DoCastVictim(SPELL_WIDE_SWIPE);
                SwipeTimer = urand(25000, 30000);
            }
            else
                SwipeTimer -= diff;
        }
    };
};

/**********************************************/
/******** Opera Romeo and Juliet Event* ******/
/********************************************/

enum JulianneRomulo
{
    /**** Speech *****/
    SAY_JULIANNE_AGGRO              = 0,
    SAY_JULIANNE_ENTER              = 1,
    SAY_JULIANNE_DEATH01            = 2,
    SAY_JULIANNE_DEATH02            = 3,
    SAY_JULIANNE_RESURRECT          = 4,
    SAY_JULIANNE_SLAY               = 5,

    SAY_ROMULO_AGGRO                = 0,
    SAY_ROMULO_DEATH                = 1,
    SAY_ROMULO_ENTER                = 2,
    SAY_ROMULO_RESURRECT            = 3,
    SAY_ROMULO_SLAY                 = 4,

    SPELL_BLINDING_PASSION          = 30890,
    SPELL_DEVOTION                  = 30887,
    SPELL_ETERNAL_AFFECTION         = 30878,
    SPELL_POWERFUL_ATTRACTION       = 30889,
    SPELL_DRINK_POISON              = 30907,

    SPELL_BACKWARD_LUNGE            = 30815,
    SPELL_DARING                    = 30841,
    SPELL_DEADLY_SWATHE             = 30817,
    SPELL_POISON_THRUST             = 30822,

    SPELL_UNDYING_LOVE              = 30951,
    SPELL_RES_VISUAL                = 24171,

    CREATURE_ROMULO                 = 17533,
    ROMULO_X                        = -10900,
    ROMULO_Y                        = -1758,
};

enum RAJPhase
{
    PHASE_JULIANNE      = 0,
    PHASE_ROMULO        = 1,
    PHASE_BOTH          = 2,
};

void PretendToDie(Creature* creature)
{
    creature->InterruptNonMeleeSpells(true);
    creature->RemoveAllAuras();
    creature->SetHealth(0);
    creature->SetUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    creature->GetMotionMaster()->MovementExpired(false);
    creature->GetMotionMaster()->MoveIdle();
    creature->SetStandState(UNIT_STAND_STATE_DEAD);
}

void Resurrect(Creature* target)
{
    target->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    target->SetFullHealth();
    target->SetStandState(UNIT_STAND_STATE_STAND);
    target->CastSpell(target, SPELL_RES_VISUAL, true);
    if (target->GetVictim())
    {
        target->GetMotionMaster()->MoveChase(target->GetVictim());
        target->AI()->AttackStart(target->GetVictim());
    }
    else
        target->GetMotionMaster()->Initialize();
}

class boss_julianne : public CreatureScript
{
public:
    boss_julianne() : CreatureScript("boss_julianne") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetKarazhanAI<boss_julianneAI>(creature);
    }

    struct boss_julianneAI : public ScriptedAI
    {
        boss_julianneAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            EntryYellTimer = 1000;
            AggroYellTimer = 10000;
            IsFakingDeath = false;
        }

        InstanceScript* instance;

        uint32 EntryYellTimer;
        uint32 AggroYellTimer;

        ObjectGuid RomuloGUID;

        uint32 Phase;

        uint32 BlindingPassionTimer;
        uint32 DevotionTimer;
        uint32 EternalAffectionTimer;
        uint32 PowerfulAttractionTimer;
        uint32 SummonRomuloTimer;
        uint32 ResurrectTimer;
        uint32 DrinkPoisonTimer;
        uint32 ResurrectSelfTimer;

        bool IsFakingDeath;
        bool SummonedRomulo;
        bool RomuloDead;

        void Reset() override
        {
            RomuloGUID.Clear();
            Phase = PHASE_JULIANNE;

            BlindingPassionTimer = 30000;
            DevotionTimer = 15000;
            EternalAffectionTimer = 25000;
            PowerfulAttractionTimer = 5000;
            SummonRomuloTimer = 10000;
            DrinkPoisonTimer = 0;
            ResurrectSelfTimer = 0;

            if (IsFakingDeath)
            {
                Resurrect(me);
                IsFakingDeath = false;
            }

            SummonedRomulo = false;
            RomuloDead = false;
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            DoZoneInCombat();
        }

        void AttackStart(Unit* who) override
        {
            if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                return;

            ScriptedAI::AttackStart(who);
        }

        void MoveInLineOfSight(Unit* who) override

        {
            if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                return;

            ScriptedAI::MoveInLineOfSight(who);
        }

        void JustReachedHome() override
        {
            me->DespawnOrUnsummon();
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* Spell) override
        {
            if (Spell->Id == SPELL_DRINK_POISON)
            {
                Talk(SAY_JULIANNE_DEATH01);
                DrinkPoisonTimer = 2500;
            }
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override;

        void EnterEvadeMode(EvadeReason reason) override
        {
            ScriptedAI::EnterEvadeMode(reason);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
        }

        void JustDied(Unit*) override
        {
            Talk(SAY_JULIANNE_DEATH02);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, DONE);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_JULIANNE_SLAY);
        }

        void UpdateAI(uint32 diff) override;
    };
};

class boss_romulo : public CreatureScript
{
public:
    boss_romulo() : CreatureScript("boss_romulo") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetKarazhanAI<boss_romuloAI>(creature);
    }

    struct boss_romuloAI : public ScriptedAI
    {
        boss_romuloAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            EntryYellTimer = 8000;
            AggroYellTimer = 15000;
        }

        InstanceScript* instance;

        ObjectGuid JulianneGUID;
        uint32 Phase;

        uint32 EntryYellTimer;
        uint32 AggroYellTimer;
        uint32 BackwardLungeTimer;
        uint32 DaringTimer;
        uint32 DeadlySwatheTimer;
        uint32 PoisonThrustTimer;
        uint32 ResurrectTimer;

        bool IsFakingDeath;
        bool JulianneDead;

        void Reset() override
        {
            JulianneGUID.Clear();
            Phase = PHASE_ROMULO;

            BackwardLungeTimer = 15000;
            DaringTimer = 20000;
            DeadlySwatheTimer = 25000;
            PoisonThrustTimer = 10000;
            ResurrectTimer = 10000;

            IsFakingDeath = false;
            JulianneDead = false;
        }

        void JustReachedHome() override
        {
            me->DespawnOrUnsummon();
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override
        {
            if (damage < me->GetHealth())
                return;

            //anything below only used if incoming damage will kill

            if (Phase == PHASE_ROMULO)
            {
                Talk(SAY_ROMULO_DEATH);
                PretendToDie(me);
                IsFakingDeath = true;
                Phase = PHASE_BOTH;

                if (Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID)))
                {
                    CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->RomuloDead = true;
                    CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->ResurrectSelfTimer = 10000;
                }

                damage = 0;
                return;
            }

            if (Phase == PHASE_BOTH)
            {
                if (JulianneDead)
                {
                    if (Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID)))
                    {
                        Julianne->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                        Julianne->GetMotionMaster()->Clear();
                        Julianne->setDeathState(JUST_DIED);
                        Julianne->CombatStop(true);
                        Julianne->GetThreatMgr().ClearAllThreat();
                        Julianne->ReplaceAllDynamicFlags(UNIT_DYNFLAG_LOOTABLE);
                    }
                    return;
                }

                if (Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID)))
                {
                    PretendToDie(me);
                    IsFakingDeath = true;
                    CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->ResurrectTimer = 10000;
                    CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->RomuloDead = true;
                    damage = 0;
                    return;
                }
            }

            //LOG_ERROR("scripts", "boss_romuloAI: DamageTaken reach end of code, that should not happen.");
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            DoZoneInCombat();
            Talk(SAY_ROMULO_AGGRO);
            if (JulianneGUID)
            {
                Creature* Julianne = ObjectAccessor::GetCreature(*me, JulianneGUID);
                if (Julianne && Julianne->GetVictim())
                {
                    me->AddThreat(Julianne->GetVictim(), 1.0f);
                    AttackStart(Julianne->GetVictim());
                }
            }
        }

        void MoveInLineOfSight(Unit* who) override

        {
            if (me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                return;

            ScriptedAI::MoveInLineOfSight(who);
        }

        void EnterEvadeMode(EvadeReason reason) override
        {
            ScriptedAI::EnterEvadeMode(reason);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, FAIL);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_ROMULO_DEATH);

            instance->SetBossState(DATA_OPERA_PERFORMANCE, DONE);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_ROMULO_SLAY);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || IsFakingDeath)
                return;

            if (JulianneDead)
            {
                if (ResurrectTimer <= diff)
                {
                    Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
                    if (Julianne && CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->IsFakingDeath)
                    {
                        Talk(SAY_ROMULO_RESURRECT);
                        Resurrect(Julianne);
                        CAST_AI(boss_julianne::boss_julianneAI, Julianne->AI())->IsFakingDeath = false;
                        JulianneDead = false;
                        ResurrectTimer = 10000;
                    }
                }
                else
                    ResurrectTimer -= diff;
            }

            if (BackwardLungeTimer <= diff)
            {
                Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 100, true);
                if (target && !me->HasInArc(M_PI, target))
                {
                    DoCast(target, SPELL_BACKWARD_LUNGE);
                    BackwardLungeTimer = urand(15000, 30000);
                }
            }
            else
                BackwardLungeTimer -= diff;

            if (DaringTimer <= diff)
            {
                DoCast(me, SPELL_DARING);
                DaringTimer = urand(20000, 40000);
            }
            else
                DaringTimer -= diff;

            if (DeadlySwatheTimer <= diff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                    DoCast(target, SPELL_DEADLY_SWATHE);
                DeadlySwatheTimer = urand(15000, 25000);
            }
            else
                DeadlySwatheTimer -= diff;

            if (PoisonThrustTimer <= diff)
            {
                DoCastVictim(SPELL_POISON_THRUST);
                PoisonThrustTimer = urand(10000, 20000);
            }
            else
                PoisonThrustTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

void boss_julianne::boss_julianneAI::UpdateAI(uint32 diff)
{
    if (EntryYellTimer)
    {
        if (EntryYellTimer <= diff)
        {
            Talk(SAY_JULIANNE_ENTER);
            EntryYellTimer = 0;
        }
        else
            EntryYellTimer -= diff;
    }

    if (AggroYellTimer)
    {
        if (AggroYellTimer <= diff)
        {
            Talk(SAY_JULIANNE_AGGRO);
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetFaction(FACTION_MONSTER_2);
            AggroYellTimer = 0;
        }
        else
            AggroYellTimer -= diff;
    }

    if (DrinkPoisonTimer)
    {
        //will do this 2secs after spell hit. this is time to display visual as expected
        if (DrinkPoisonTimer <= diff)
        {
            PretendToDie(me);
            Phase = PHASE_ROMULO;
            SummonRomuloTimer = 10000;
            DrinkPoisonTimer = 0;
        }
        else
            DrinkPoisonTimer -= diff;
    }

    if (Phase == PHASE_ROMULO && !SummonedRomulo)
    {
        if (SummonRomuloTimer <= diff)
        {
            if (Creature* pRomulo = me->SummonCreature(CREATURE_ROMULO, ROMULO_X, ROMULO_Y, me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, HOUR * 2 * IN_MILLISECONDS))
            {
                RomuloGUID = pRomulo->GetGUID();
                CAST_AI(boss_romulo::boss_romuloAI, pRomulo->AI())->JulianneGUID = me->GetGUID();
                CAST_AI(boss_romulo::boss_romuloAI, pRomulo->AI())->Phase = PHASE_ROMULO;
                DoZoneInCombat(pRomulo);

                pRomulo->SetFaction(FACTION_MONSTER_2);
            }
            SummonedRomulo = true;
        }
        else
            SummonRomuloTimer -= diff;
    }

    if (ResurrectSelfTimer)
    {
        if (ResurrectSelfTimer <= diff)
        {
            Resurrect(me);
            Phase = PHASE_BOTH;
            IsFakingDeath = false;

            if (me->GetVictim())
                AttackStart(me->GetVictim());

            ResurrectSelfTimer = 0;
            ResurrectTimer = 1000;
        }
        else
            ResurrectSelfTimer -= diff;
    }

    if (!UpdateVictim() || IsFakingDeath)
        return;

    if (RomuloDead)
    {
        if (ResurrectTimer <= diff)
        {
            Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID));
            if (Romulo && CAST_AI(boss_romulo::boss_romuloAI, Romulo->AI())->IsFakingDeath)
            {
                Talk(SAY_JULIANNE_RESURRECT);
                Resurrect(Romulo);
                CAST_AI(boss_romulo::boss_romuloAI, Romulo->AI())->IsFakingDeath = false;
                RomuloDead = false;
                ResurrectTimer = 10000;
            }
        }
        else
            ResurrectTimer -= diff;
    }

    if (BlindingPassionTimer <= diff)
    {
        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
            DoCast(target, SPELL_BLINDING_PASSION);
        BlindingPassionTimer = urand(30000, 45000);
    }
    else
        BlindingPassionTimer -= diff;

    if (DevotionTimer <= diff)
    {
        DoCast(me, SPELL_DEVOTION);
        DevotionTimer = urand(15000, 45000);
    }
    else
        DevotionTimer -= diff;

    if (PowerfulAttractionTimer <= diff)
    {
        DoCast(SelectTarget(SelectTargetMethod::Random, 0), SPELL_POWERFUL_ATTRACTION);
        PowerfulAttractionTimer = urand(5000, 30000);
    }
    else
        PowerfulAttractionTimer -= diff;

    if (EternalAffectionTimer <= diff)
    {
        if (urand(0, 1) && SummonedRomulo)
        {
            Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID));
            if (Romulo && Romulo->IsAlive() && !RomuloDead)
                DoCast(Romulo, SPELL_ETERNAL_AFFECTION);
        }
        else DoCast(me, SPELL_ETERNAL_AFFECTION);

        EternalAffectionTimer = urand(45000, 60000);
    }
    else
        EternalAffectionTimer -= diff;

    DoMeleeAttackIfReady();
}

void boss_julianne::boss_julianneAI::DamageTaken(Unit* /*done_by*/, uint32& damage, DamageEffectType, SpellSchoolMask)
{
    if (damage < me->GetHealth())
        return;

    //anything below only used if incoming damage will kill

    if (Phase == PHASE_JULIANNE)
    {
        damage = 0;

        //this means already drinking, so return
        if (IsFakingDeath)
            return;

        me->InterruptNonMeleeSpells(true);
        DoCast(me, SPELL_DRINK_POISON);

        IsFakingDeath = true;
        //IS THIS USEFULL? Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
        return;
    }

    if (Phase == PHASE_ROMULO)
    {
        //LOG_ERROR("scripts", "boss_julianneAI: cannot take damage in PHASE_ROMULO, why was i here?");
        damage = 0;
        return;
    }

    if (Phase == PHASE_BOTH)
    {
        //if this is true then we have to kill romulo too
        if (RomuloDead)
        {
            if (Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID)))
            {
                Romulo->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                Romulo->GetMotionMaster()->Clear();
                Romulo->setDeathState(JUST_DIED);
                Romulo->CombatStop(true);
                Romulo->GetThreatMgr().ClearAllThreat();
                Romulo->ReplaceAllDynamicFlags(UNIT_DYNFLAG_LOOTABLE);
            }

            return;
        }

        //if not already returned, then romulo is alive and we can pretend die
        if (Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID)))
        {
            PretendToDie(me);
            IsFakingDeath = true;
            CAST_AI(boss_romulo::boss_romuloAI, Romulo->AI())->ResurrectTimer = 10000;
            CAST_AI(boss_romulo::boss_romuloAI, Romulo->AI())->JulianneDead = true;
            damage = 0;
            return;
        }
    }
    //LOG_ERROR("scripts", "boss_julianneAI: DamageTaken reach end of code, that should not happen.");
}

void AddSC_bosses_opera()
{
    RegisterKarazhanCreatureAI(boss_dorothee);
    RegisterKarazhanCreatureAI(boss_strawman);
    RegisterKarazhanCreatureAI(boss_tinhead);
    RegisterKarazhanCreatureAI(boss_roar);
    RegisterKarazhanCreatureAI(boss_crone);
    RegisterKarazhanCreatureAI(npc_tito);
    RegisterKarazhanCreatureAI(npc_cyclone);
    new npc_grandmother();
    new boss_bigbadwolf();
    new boss_julianne();
    new boss_romulo();
}
