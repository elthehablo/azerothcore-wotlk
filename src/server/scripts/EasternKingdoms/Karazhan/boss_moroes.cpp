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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "karazhan.h"

enum Yells
{
    SAY_AGGRO                   = 0,
    SAY_SPECIAL                 = 1,
    SAY_KILL                    = 2,
    SAY_DEATH                   = 3,
    SAY_OUT_OF_COMBAT           = 4, //defunct?

    SAY_GUEST                   = 0
};

enum Spells
{
    SPELL_VANISH                = 29448,
    SPELL_GARROTE_DUMMY         = 29433,
    SPELL_GARROTE               = 37066,
    SPELL_BLIND                 = 34694,
    SPELL_GOUGE                 = 29425,
    SPELL_FRENZY                = 37023,
    SPELL_DUAL_WIELD            = 29651,
    SPELL_BERSERK               = 26662,
    SPELL_VANISH_TELEPORT       = 29431,
};

enum Misc
{
    ACTIVE_GUEST_COUNT          = 4,
    MAX_GUEST_COUNT             = 6
};

enum Groups
{
    GROUP_BEFORE_COMBAT         = 0
};

const Position GuestsPosition[4] =
{
    {-10987.38f, -1883.38f, 81.73f, 1.50f},
    {-10989.60f, -1881.27f, 81.73f, 0.73f},
    {-10978.81f, -1884.08f, 81.73f, 1.50f},
    {-10976.38f, -1882.59f, 81.73f, 2.31f},
};

const uint32 GuestEntries[6] =
{
    17007,
    19872,
    19873,
    19874,
    19875,
    19876,
};

struct boss_moroes : public BossAI
{
    boss_moroes(Creature* creature) : BossAI(creature, DATA_MOROES)
    {
        _activeGuests = 0;
        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });
    }

    void InitializeAI() override
    {
        BossAI::InitializeAI();
        InitializeGuests();
    }

    void JustReachedHome() override
    {
        BossAI::JustReachedHome();
        InitializeGuests();
    }

    void InitializeGuests()
    {
        if (!me->IsAlive())
            return;

        if (_activeGuests == 0)
        {
            _activeGuests |= 0x3F;
            uint8 rand1 = RAND(0x01, 0x02, 0x04);
            uint8 rand2 = RAND(0x08, 0x10, 0x20);
            _activeGuests &= ~(rand1 | rand2);
        }

        for (uint8 i = 0; i < MAX_GUEST_COUNT; ++i)
            if ((1 << i) & _activeGuests)
                me->SummonCreature(GuestEntries[i], GuestsPosition[summons.size()], TEMPSUMMON_MANUAL_DESPAWN);

        scheduler.CancelGroup(GROUP_BEFORE_COMBAT);
        scheduler.Schedule(10s, GROUP_BEFORE_COMBAT, [this](TaskContext context)
        {
            if (Creature* guest = GetRandomGuest())
                guest->AI()->Talk(SAY_GUEST);
            context.Repeat(5s);
        });
    }

    void Reset() override
    {
        _Reset();
        _recentlySpoken = false;
        _currentPhase = 0;
        DoCastSelf(SPELL_DUAL_WIELD, true);

        ScheduleHealthCheckEvent(31, [&]{
            DoCastSelf(SPELL_FRENZY, true);
        });
    }

    void JustEngagedWith(Unit* who) override
    {
        _JustEngagedWith();
        Talk(SAY_AGGRO);


        scheduler.Schedule(30s, [this](TaskContext context)
        {
                scheduler.DelayAll(9s);
                _currentPhase = 1;
                DoCastSelf(SPELL_VANISH);
                context.Repeat(30s);
                scheduler.Schedule(5s, 7s, [this](TaskContext)
                {
                    Talk(SAY_SPECIAL);
                    DoCastRandomTarget(SPELL_GARROTE, 0, 100.0f, true, true);
                    DoCastSelf(SPELL_VANISH_TELEPORT);
                    _currentPhase = 0;
                });
        }).Schedule(20s, [this](TaskContext context)
        {
            if (Unit* target = SelectTarget(SelectTargetMethod::MaxThreat, 1, 10.0f, true))
            {
                DoCast(target, SPELL_BLIND);
            }
            context.Repeat(25s, 40s);
        }).Schedule(13s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_GOUGE);
            context.Repeat(25s, 40s);
        }).Schedule(10min, [this](TaskContext)
        {
            DoCastSelf(SPELL_BERSERK, true);
        });

        scheduler.CancelGroup(GROUP_BEFORE_COMBAT);
        me->CallForHelp(20.0f);
        DoZoneInCombat();
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        if(!_recentlySpoken)
        {
            Talk(SAY_KILL);
            _recentlySpoken = true;
        }

        scheduler.Schedule(5s, [this](TaskContext)
        {
            _recentlySpoken = false;
        });
    }

    void JustDied(Unit* killer) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GARROTE);
    }

    Creature* GetRandomGuest()
    {
        std::list<Creature*> guestList;
        for (SummonList::const_iterator i = summons.begin(); i != summons.end(); ++i)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, *i))
                guestList.push_back(summon);

        return Acore::Containers::SelectRandomContainerElement(guestList);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        scheduler.Update(diff);

        // Xinef: not in vanish
        if (_currentPhase == 0)
            DoMeleeAttackIfReady();
    }

    private:
        uint8 _activeGuests;
        bool _recentlySpoken;
        uint8 _currentPhase;
};

class spell_moroes_vanish : public SpellScriptLoader
{
public:
    spell_moroes_vanish() : SpellScriptLoader("spell_moroes_vanish") { }

    class spell_moroes_vanish_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_moroes_vanish_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);
            if (Unit* target = GetHitUnit())
            {
                Position pos = target->GetFirstCollisionPosition(5.0f, M_PI);
                GetCaster()->CastSpell(target, SPELL_GARROTE_DUMMY, true);
                GetCaster()->RemoveAurasDueToSpell(SPELL_VANISH);
                GetCaster()->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), target->GetOrientation());
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_moroes_vanish_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_moroes_vanish_SpellScript();
    }
};

void AddSC_boss_moroes()
{
    RegisterKarazhanCreatureAI(boss_moroes);
    new spell_moroes_vanish();
}
