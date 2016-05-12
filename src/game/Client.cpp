#include <bgame/impl.h>
#include <omnibot/et/g_etbot_interface.h>

///////////////////////////////////////////////////////////////////////////////

extern gclient_t g_clients[];

void GibEntity( gentity_t *self, int killer );

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

int __nextSlot = 0;

///////////////////////////////////////////////////////////////////////////////

class _BufferPrint : public stringbuf
{
private:
    Client& _client;

protected:
    int
    sync()
    {
        text::Buffer buf( text::xcdebug );
        buf << str();
        _client.xprint( buf );
        str( "" );
        return 0;
    }

public:
    _BufferPrint( Client& client_ )
        : stringbuf ( )
        , _client   ( client_ )
    {
    }

    ~_BufferPrint()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

class _BufferDebug : public stringbuf
{
private:
    Client& _client;

protected:
    int
    sync()
    {
        text::Buffer buf( text::xcdebug );
        buf << str();
        _client.xprint( buf );
        str( "" );
        return 0;
    }

public:
    _BufferDebug( Client& client_ )
        : stringbuf ( )
        , _client   ( client_ )
    {
    }

    ~_BufferDebug()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

Client::Client()
    : _needGreeting   ( false )
    , _numNameChanges ( 0 )
    , slot            ( __nextSlot++ )
    , debug           ( new _BufferDebug( *this ))
    , print           ( new _BufferPrint( *this ))
    , gclient         ( g_clients[slot] )
    , gentity         ( g_entities[slot] )
    , bulletModel     ( 0 )
    , hitModel        ( 0 )
    , numNameChanges  ( _numNameChanges )
    , cmdCount        ( 0 )
    , cmdDelta        ( 0 )
    , cmdLastRealTime ( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////

Client::~Client()
{
    delete debug.rdbuf();
    delete print.rdbuf();
}

///////////////////////////////////////////////////////////////////////////////

int
Client::calculateKnockback(int damage, vec3_t dir)
{
    int knockback = damage;

    if (knockback > 200)
        knockback = 200;

    if (gentity.flags & FL_NO_KNOCKBACK)
        knockback = 0;

    // ydnar: set weapons means less knockback
    if (gclient.ps.weapon == WP_MORTAR_SET || gclient.ps.weapon == WP_MOBILE_MG42_SET)
        knockback = int( knockback * 0.5f );

    // figure momentum add, even if the damage won't be taken
    if (knockback) {
        vec3_t  kvel;
        float   mass;

        mass = 200;

        VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
        VectorAdd (gclient.ps.velocity, kvel, gclient.ps.velocity);

        // set the timer so that the other client can't cancel
        // out the movement immediately
        if (!gclient.ps.pm_time) {
            int t;

            t = knockback * 2;
            if (t < 50)
                t = 50;
            else if (t > 200)
                t = 200;

            gclient.ps.pm_time = t;
            gclient.ps.pm_flags |= PMF_TIME_KNOCKBACK;
        }
    }

    return knockback;
}

///////////////////////////////////////////////////////////////////////////////

bool
Client::headshotAllowed(const int mod)
{
    switch (mod) {
        case MOD_M97:
            return false;

        default:
            return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::init()
{
    _numNameChanges = 0;

    memset( &gclient, 0, sizeof(gclient) );

    delete bulletModel;
    AbstractBulletModel::factory(
        bulletModel,
        *this,
        static_cast< AbstractBulletModel::type_t >( cvars::bg_bulletmode.ivalue ),
        true );

    delete hitModel;
    AbstractHitModel::factory(
        hitModel,
        *this,
        static_cast< AbstractHitModel::type_t >( cvars::bg_hitmode.ivalue ),
        AbstractHitModel::VITALITY_PRINCIPAL );

    cmdQueue.clear();
    cmdCount = 0;
    cmdDelta = 0;
    cmdLastRealTime = 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
Client::isReconcileSafe()
{
    if (!gentity.r.linked)
        return false;
  
    if (gclient.ps.pm_flags & PMF_LIMBO)
        return false;

    switch (gclient.sess.sessionTeam) {
        case TEAM_ALLIES:
        case TEAM_AXIS:
            break;

        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
Client::isVisibleTarget()
{
    if (!gentity.r.linked)
        return false;
 
    if (gclient.ps.pm_flags & PMF_LIMBO)
        return false;

    switch (gclient.sess.sessionTeam) {
        case TEAM_ALLIES:
        case TEAM_AXIS:
            break;

        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Client::addStats(AbstractHitVolume::zone_t zone, int mod)
{
    unsigned int weap = G_weapStatIndex_MOD(mod);

    switch (zone) {
        case AbstractHitVolume::ZONE_HEAD:
            gclient.sess.aWeaponStats[weap].headshots++;
                gclient.sess.headshots++;               // Jaybird - track these as well for end-game stats
            break;

        case AbstractHitVolume::ZONE_ARM_LEFT:
        case AbstractHitVolume::ZONE_HAND_LEFT:
            gclient.sess.aWeaponStats[weap].leftarmshots++;
            break;

        case AbstractHitVolume::ZONE_ARM_RIGHT:
        case AbstractHitVolume::ZONE_HAND_RIGHT:
            gclient.sess.aWeaponStats[weap].rightarmshots++;
            break;

        case AbstractHitVolume::ZONE_LEGS:
            gclient.sess.aWeaponStats[weap].rightlegshots++;
            gclient.sess.aWeaponStats[weap].leftlegshots++;
            break;

        case AbstractHitVolume::ZONE_LEG_LEFT:
        case AbstractHitVolume::ZONE_FOOT_LEFT:
            gclient.sess.aWeaponStats[weap].leftlegshots++;
            break;

        case AbstractHitVolume::ZONE_LEG_RIGHT:
        case AbstractHitVolume::ZONE_FOOT_RIGHT:
            gclient.sess.aWeaponStats[weap].rightlegshots++;
            break;

        case AbstractHitVolume::ZONE_BODY:
        case AbstractHitVolume::ZONE_TORSO:
        case AbstractHitVolume::ZONE_TORSO_LEFT:
        case AbstractHitVolume::ZONE_TORSO_RIGHT:
        default:
            gclient.sess.aWeaponStats[weap].bodyshots++;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::notifyBegin()
{
    if (_needGreeting) {
        _needGreeting = false;
        greeting();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::notifyConnecting( bool firstTime )
{
    if (firstTime) {
        _needGreeting = true;

        // clear any reports which may be left over from last player
        using namespace text;

        vector<Buffer*>& report = cmd::Page::clientReports[slot];

        const vector<Buffer*>::size_type max = report.size();
        for ( vector<Buffer*>::size_type i = 0; i < max; i++ )
            delete report[i];
        report.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::notifyNameChanged()
{
    _numNameChanges++;
}

///////////////////////////////////////////////////////////////////////////////

void
Client::recordHit( AbstractHitVolume::zone_t zone, bool friendly )
{
    const holdable_t h = friendly ? HOLDABLE_HITF : HOLDABLE_HIT;

    int mask;
    int bump;
    switch (zone) {
        case AbstractHitVolume::ZONE_HEAD:
            mask = 0x000f;
            bump = 0x0001;
            break;

        case AbstractHitVolume::ZONE_ARM_LEFT:
        case AbstractHitVolume::ZONE_ARM_RIGHT:
        case AbstractHitVolume::ZONE_HAND_LEFT:
        case AbstractHitVolume::ZONE_HAND_RIGHT:
            mask = 0x00f0;
            bump = 0x0010;
            break;

        default:
        case AbstractHitVolume::ZONE_BODY:
        case AbstractHitVolume::ZONE_TORSO:
        case AbstractHitVolume::ZONE_TORSO_LEFT:
        case AbstractHitVolume::ZONE_TORSO_RIGHT:
            mask = 0x0f00;
            bump = 0x0100;
            break;

        case AbstractHitVolume::ZONE_LEGS:
        case AbstractHitVolume::ZONE_LEG_LEFT:
        case AbstractHitVolume::ZONE_LEG_RIGHT:
        case AbstractHitVolume::ZONE_FOOT_LEFT:
        case AbstractHitVolume::ZONE_FOOT_RIGHT:
            mask = 0xf000;
            bump = 0x1000;
            break;
    }

if (!(cvars::g_test.ivalue & G_TEST_SKIP_HITSOUNDS))
    gclient.ps.holdable[h] = (gclient.ps.holdable[h] & ~mask) | ((((gclient.ps.holdable[h] & mask) + bump) & mask));
}

///////////////////////////////////////////////////////////////////////////////

void
Client::reset()
{
    shutdown();
    init();
}

///////////////////////////////////////////////////////////////////////////////

void
Client::run()
{
    ClientEndFrame( &gentity );

    ///////////////////////////////////////////////////////////////
    //
    // bullet-model section
    //
    ///////////////////////////////////////////////////////////////

    // It does not make a difference whether bullet-model or hit-model is
    // run first because bullet-model does not do hit checks or reference
    // hit-model from run().

    if (cvars::bg_bulletmode.ivalue != bulletModel->type) {
        delete bulletModel;
        if ( AbstractBulletModel::factory(
                bulletModel,
                *this,
                static_cast< AbstractBulletModel::type_t >( cvars::bg_bulletmode.ivalue ),
                true ))
        {
            // Cvar value must be invalid but factory gave us default implementation,
            // thus cvar should be updated to reflect active type.
            cvars::bg_bulletmode.set( bulletModel->type );
        }
    }

    bulletModel->run();

    ///////////////////////////////////////////////////////////////
    //
    // hit-model section
    //
    ///////////////////////////////////////////////////////////////

    if (cvars::bg_hitmode.ivalue != hitModel->type) {
        delete hitModel;
        if ( AbstractHitModel::factory(
                hitModel,
                *this,
                static_cast< AbstractHitModel::type_t >( cvars::bg_hitmode.ivalue ),
                AbstractHitModel::VITALITY_PRINCIPAL ))
        {
            // Cvar value must be invalid but factory gave us default implementation,
            // thus cvar should be updated to reflect active type.
            cvars::bg_hitmode.set( hitModel->type );
        }
    }

    hitModel->run();
}

///////////////////////////////////////////////////////////////////////////////

void
Client::shutdown()
{
    delete hitModel;
    hitModel = 0;

    delete bulletModel;
    bulletModel = 0;
}

///////////////////////////////////////////////////////////////////////////////

void
Client::takeBulletDamageFrom( const TraceContext& trx, Client& actor, int damage, bool distanceFalloff, const int mod )
{
    if (level.intermissionQueued || (cvars::gameState.ivalue != GS_PLAYING && !match_warmupDamage.integer))
        return;

    const bool onSameTeam = gclient.sess.sessionTeam == actor.gclient.sess.sessionTeam;

    if (damage < 1)
        damage = 1;

    int take = damage;

    // Cut down damage when on adrenaline.
    if (gclient.ps.powerups[PW_ADRENALINE])
        take = int( take * 0.5f );

    // Cut down damage when wearing flak jacket.
    if ( gclient.sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 4 &&
         (gclient.sess.playerType == PC_ENGINEER || (cvars::bg_skills.ivalue & SBS_ENGI) ))
    {
        switch (mod) {
            case MOD_AIRSTRIKE:
            case MOD_ARTY:
            case MOD_DYNAMITE:
            case MOD_EXPLOSIVE:
            case MOD_GPG40:
            case MOD_GRENADE:
            case MOD_GRENADE_LAUNCHER:
            case MOD_GRENADE_PINEAPPLE:
            case MOD_LANDMINE:
            case MOD_M7:
            case MOD_MAPMORTAR:
            case MOD_MAPMORTAR_SPLASH: 
            case MOD_MORTAR:
            case MOD_PANZERFAUST:
            case MOD_ROCKET:
            case MOD_SATCHEL:
                take -= int( take * 0.5f );
                break;

            default:
                break;
        }
    }

    const bool isHeadShot = trx.hitvol->zone == AbstractHitVolume::ZONE_HEAD;
    const bool wasAlive = gentity.health > 0;
    const hitRegion_t hr = isHeadShot ? HR_HEAD : HR_BODY;

    // Fast-exit if not headshot and headshot-only mode.
    if (!isHeadShot && (g_headshot.integer & HEADSHOT_ONLY))
        return;

    // Bump hitsound counters.
    if (g_friendlyFire.integer || !onSameTeam)
        actor.recordHit( trx.hitvol->zone, onSameTeam );

    // Fast-exit if god.
    if (gclient.noclip || gclient.ps.powerups[PW_INVULNERABLE] || gentity.flags & FL_GODMODE)
        return;

    // Fast-exit if friendly-fire and friendly-fire is disabled.
    if (onSameTeam && !g_friendlyFire.integer)
        return;

    if (gentity.health > 0) {
        // vsay "hold your fire" on the first hit of a teammate
        // only applies if the player has been hurt before
        // and the match is not in warmup.
        if (isHeadShot && cvars::bg_sniperWar.ivalue && ( mod == MOD_K43_SCOPE || mod == MOD_GARAND_SCOPE )) {
            G_GlobalClientEvent( EV_HEADSHOT, 0, actor.slot );
        }
        else if (g_friendlyFire.integer && onSameTeam) {
            if ( (!gclient.lasthurt_mod || gclient.lasthurt_client != actor.slot ) &&
                 cvars::gameState.ivalue == GS_PLAYING &&
                 (actor.gentity.health - take) > FORCE_LIMBO_HEALTH )
            {
                if (gclient.sess.sessionTeam == TEAM_AXIS)
                    G_ClientSound( &actor.gentity, "sound/chat/axis/26a.wav" );
                else    
                    G_ClientSound( &actor.gentity, "sound/chat/allies/26a.wav" );
            }
        }
    }

    // Update combat-state.
    if (!onSameTeam && cvars::gameState.ivalue == GS_PLAYING) {
        gclient.combatState = combatstate_t( gclient.combatState | ( 1 << COMBATSTATE_DAMAGERECEIVED ));
        actor.gclient.combatState = combatstate_t( actor.gclient.combatState | ( 1 << COMBATSTATE_DAMAGEDEALT ));
    }

    vec3_t dir;
    VectorSubtract( trx.end, trx.start, dir );
    VectorNormalizeFast( dir );

    // Knockback
    int knockback = 0;
    if (!g_friendlyFire.integer || !onSameTeam)
        knockback = calculateKnockback(damage, dir);

    if (isHeadShot) {
        if (headshotAllowed(mod)) {
            if (take * 2 < 50) // head shots, all weapons, do minimum 50 points damage
                take = 50;
            else
                take *= 2; // sniper rifles can do full-kill (and knock into limbo)
        }

        if (distanceFalloff) {
            take = G_DistanceFalloffAdjustment(take, trx.flen, 0.2f);
        }

        if ( !(gclient.ps.eFlags & EF_HEADSHOT) ) {
            // only toss hat on first headshot
            G_AddEvent( &gentity, EV_LOSE_HAT, DirToByte( dir ));

            if (mod != MOD_K43_SCOPE && mod != MOD_GARAND_SCOPE)
                take = int( take * 0.8f );  // helmet gives us some protection
        }

        // Sniper war instant kill
        if (cvars::bg_sniperWar.ivalue) {
            if (mod == MOD_K43_SCOPE || mod == MOD_GARAND_SCOPE)
                take = (GIB_HEALTH * -1) - 1; // Jaybird - code below prevents this from gibbing.
        }

        // Jaybird - g_headshot
        if (g_headshot.integer & HEADSHOT_INSTAGIB)
            take = (GIB_HEALTH * -1) - 1; // Jaybird - code below prevents this from gibbing.

        gclient.ps.eFlags |= EF_HEADSHOT;

        // OSP - Record the headshot
        //G_addStatsHeadShot( &actor.gentity, mod );
    }

    // Jaybird - add stats for logging
    actor.addStats(trx.hitvol->zone, mod);

    // Reflected friendly fire
        if( gentity.health > 0 && onSameTeam && g_friendlyFire.integer == 2 && IsReflectable( mod )) {
                int ffDamage;

                // Percentage based reflect
                ffDamage = int( take * g_reflectFriendlyFire.value / 100.f );
                if( ffDamage <= 0 ) {
                        ffDamage = 0;
                }
                actor.gentity.health -= ffDamage;

                // Give them pain!
                actor.gclient.damage_blood += take;
                actor.gclient.damage_knockback += knockback;

                // Set the lasthurt stuff so hitsounds do not replay
                gclient.lasthurt_mod = mod;
                gclient.lasthurt_client = actor.slot;

                // Kill the player if necessary
                if( actor.gentity.health <= 0 ) {
                        actor.gentity.deathType = MOD_REFLECTED_FF;
                        actor.gentity.enemy = &actor.gentity;
                        if( actor.gentity.die ) {
                                actor.gentity.die( &actor.gentity, &actor.gentity, &actor.gentity, ffDamage, MOD_REFLECTED_FF );
                        }
                }
        }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    gclient.ps.persistant[PERS_ATTACKER] = actor.gentity.s.number;
    gclient.damage_blood += take;
    gclient.damage_knockback += knockback;

    VectorCopy( dir, gclient.damage_from );
    gclient.damage_fromWorld = qfalse;

    // set the last client who damaged the target
    gclient.lasthurt_client = actor.slot;
    gclient.lasthurt_mod    = mod;
    gclient.lasthurt_time   = level.time;

    // Fast-exit if no damage to take.
    if (take < 1)
        return;

    gentity.health -= take;

    // Ridah, can't gib with bullet weapons (except VENOM)
    // Arnout: attacker == inflictor can happen in other cases as well! (movers trying to gib things)
    if (gentity.health <= GIB_HEALTH && !G_WeaponIsExplosive( meansOfDeath_t(mod) ))
        gentity.health = GIB_HEALTH + 1;

    if (g_damagexp.integer && !onSameTeam) {
        // Jaybird - give them some per hit
        // They get 1 XP per 50 damage, so multiple .02 * take
        int skill = G_SkillForMOD( mod );
        if (skill >= 0)
            G_AddSkillPoints( &actor.gentity, skillType_t(skill), take * 0.02f );
    }

    if (take > 190)
        gentity.health = GIB_HEALTH - 1;

    if (gentity.health <= 0) {
        if (!wasAlive) {
            gentity.flags |= FL_NO_KNOCKBACK;
            // OSP - special hack to not count attempts for body gibbage
            if (gclient.ps.pm_type == PM_DEAD)
                G_addStats( &gentity, &actor.gentity, take, mod );

            if ( (gentity.health < FORCE_LIMBO_HEALTH) && (gentity.health > GIB_HEALTH) )
                limbo( &gentity, qtrue );

            // xkan, 1/13/2003 - record the time we died.
            if (gclient.deathTime < 1)
                gclient.deathTime = level.time;

            // bani - #389
            if (gentity.health <= GIB_HEALTH )
                GibEntity( &gentity, 0 );
        }
        else {
            gentity.sound1to2 = hr;
            gentity.sound2to1 = mod;
            gentity.sound2to3 = 0;

            if (!onSameTeam)
                G_AddKillSkillPoints( &actor.gentity, meansOfDeath_t(mod), hr, qfalse );

            if (gentity.health < -999)
                gentity.health = -999;

            gentity.enemy = &actor.gentity;
            gentity.deathType = meansOfDeath_t(mod);

            if (gentity.die)
                gentity.die( &gentity, &actor.gentity, &actor.gentity, take, mod );

            // if we freed ourselves in death function
            if (!gentity.inuse)
                return;

            // RF, entity scripting
            if (gentity.health <= 0) {   // might have revived itself in death function
                G_Script_ScriptEvent( &gentity, "death", "" );
            }
        }
    }
    else if (gentity.pain) {
        VectorCopy( dir, gentity.rotate );
        VectorCopy( trx.data.endpos, gentity.pos3 ); // this will pass loc of hit
        gentity.pain( &gentity, &actor.gentity, take, gentity.pos3 );
    }
    else {
        // OSP - update weapon/dmg stats
        G_addStats( &gentity, &actor.gentity, take, mod );
        // OSP
    }

    // RF, entity scripting
    G_Script_ScriptEvent( &gentity, "pain", va("%d %d", gentity.health, gentity.health+take) );

	// notify omni-bot framework
	Bot_Event_TakeDamage(&gentity-g_entities, &actor.gentity);

    // Ridah, this needs to be done last, incase the health is altered in one of the event calls
    gclient.ps.stats[STAT_HEALTH] = gentity.health;

    // Reflected friendly fire
    if (gentity.health > 0 && onSameTeam && g_friendlyFire.integer == 2 && IsReflectable( mod )) {
        int ffDamage;

        // Percentage based reflect
        ffDamage = int( take * g_reflectFriendlyFire.value / 100.f );
        if (ffDamage <= 0)
            ffDamage = 0;
        actor.gentity.health -= ffDamage;

        // Give them pain!
        actor.gclient.damage_blood += take;
        actor.gclient.damage_knockback += knockback;

        // Set the lasthurt stuff so hitsounds do not replay
        gclient.lasthurt_mod = mod;
        gclient.lasthurt_client = actor.slot;

        // Kill the player if necessary
        if( actor.gentity.health <= 0 ) {
            actor.gentity.deathType = MOD_REFLECTED_FF;
            actor.gentity.enemy = &actor.gentity;
            if( actor.gentity.die ) {
                actor.gentity.die( &actor.gentity, &actor.gentity, &actor.gentity, ffDamage, MOD_REFLECTED_FF );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::think()
{
    ClientThink( slot );
}

///////////////////////////////////////////////////////////////////////////////

void
Client::xpBackup()
{
    User& user = *connectedUsers[slot];

    if (user.fakeguid || !g_xpSave.integer)
        return;

    for (int i = 0; i < SK_NUM_SKILLS; i++)
        user.xpSkills[i] = gclient.sess.skillpoints[i];

    user.timestamp = time( NULL );
}

///////////////////////////////////////////////////////////////////////////////

void
Client::xpReset()
{
    // Reset slot stats
    trap_Cvar_Set( va( "sessionstats%i", slot ), "" );
    trap_Cvar_Set( va( "wstats%i", slot ), "" );

    // Reset rank and skills
    gclient.sess.rank = 0;
    for (int i = 0; i < SK_NUM_SKILLS; i++ ) {
        gclient.sess.skillpoints[i] = 0.0f;
        gclient.sess.skill[i] = 0;
    }

    // Reset overall stats
    G_CalcRank( &gclient );
    gclient.ps.stats[STAT_XP] = 0;
    gclient.ps.persistant[PERS_SCORE] = 0;

    // Back up current ammo
    int ammo[MAX_WEAPONS];
    int ammoclip[MAX_WEAPONS];

    memset( gclient.ps.weapons, 0, sizeof(gclient.ps.weapons) );
    memcpy( ammo, gclient.ps.ammo, sizeof(ammo) );
    memcpy( ammoclip, gclient.ps.ammoclip, sizeof(ammoclip) );

    // Jaybird - #4
    // Back up and restore weapon charge, otherwise will get full bar
    int classWeaponTime = gclient.ps.classWeaponTime;

    // Set default weapons
    SetWolfSpawnWeapons( &gclient );

    // Jaybird - #4
    gclient.ps.classWeaponTime = classWeaponTime;

    // Restore ammo
    memcpy( gclient.ps.ammo, ammo, sizeof(ammo) );
    memcpy( gclient.ps.ammoclip, ammoclip, sizeof(ammoclip) );
    ClientUserinfoChanged( slot );
}

///////////////////////////////////////////////////////////////////////////////

void
Client::xpRestore()
{
    User& user = *connectedUsers[slot];

    if (user.fakeguid || !g_xpSave.integer)
        return;

    const int timeout = str::toSeconds( g_xpSaveTimeout.string );

    if (timeout <= 0 || time(NULL) - user.timestamp < timeout) {
        // Set up total XP
        gclient.ps.stats[STAT_XP] = 0;

        // Restore individual XP levels
        for (int i = 0; i < SK_NUM_SKILLS; i++) {
            gclient.sess.skillpoints[i] = user.xpSkills[i];
            gclient.ps.stats[STAT_XP] += int( user.xpSkills[i] );
        }

        // Get Rank
        G_CalcRank( &gclient );

        BG_PlayerStateToEntityState( &gclient.ps, &gentity.s, level.time, qtrue );

        // Jaybird - also give the client some information
        struct tm* currentTime = localtime(&user.timestamp);
        char timestamp[32]; 
        strftime( timestamp, sizeof(timestamp), "%c", currentTime );
        CPx( slot, va( "chat \"^3Your XP has been restored from %s\" -1 0 4", timestamp ));
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Client::xprint( const text::Buffer& buf )
{
    trap_SendServerCommand( "xpr", buf, slot );
}

///////////////////////////////////////////////////////////////////////////////

void
Client::greeting()
{
    // Get user and level
    User& user = *connectedUsers[slot];

    string err;
    Level& lev = levelDB.fetchByKey( user.authLevel, err );
    if (lev == Level::BAD)
        return;

    // bail if no GUID
    if( user.fakeguid )
        return;

    // Handle text first
    if (user.greetingText.length() || lev.greetingText.length()) {
        static char token[MAX_TOKEN_CHARS];
        const char *s;
        char temp[MAX_STRING_CHARS];
        int i;

        if (user.greetingText.length()) {
            s = user.greetingText.c_str();
        } else {
            s = lev.greetingText.c_str();
        }

        memset( token, 0, sizeof(token) );
        i = 0;

        // Replace valid tokens
        while( i < int(sizeof(token)) && *s ) {
            if ( *s == '[' ) {
                if( !Q_strncmp( s, "[player]", 8 )) {
                    Q_strncpyz( temp, user.namex.c_str(), sizeof(temp) );
                    strncat( token, temp, sizeof(token) );
                    i += strlen( temp );
                    s += 8;
                    continue;
                }
                else if( !Q_strncmp( s, "[level]", 7 )) {
                    strncat( token, lev.name.c_str(), sizeof(token) );
                    i += lev.name.length();
                    s += 7;
                    continue;
                }
            }
            token[i] = *s;
            s++;
            i++;
            }
        AP( va( "chat \"%s\"", token ));
    }

    // Now handle audio
    if (user.greetingAudio.length() || lev.greetingAudio.length()) {
        string sound;
        if (user.greetingAudio.length()) {
            sound = user.greetingAudio;
        } else {
            sound = lev.greetingAudio;
        }
        G_globalSound((char *)sound.c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////

Client g_clientObjects[ MAX_CLIENTS ];
