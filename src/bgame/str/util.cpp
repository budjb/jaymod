#include <bgame/impl.h>

namespace str {

///////////////////////////////////////////////////////////////////////////////

string&
etAlignLeft( const string& value, const string::size_type width, string& result )
{
    string::size_type len = etLength( value );

    result = value;

    if (len < width)
        result.append( width-len, ' ' );
    else if (len > width)
        etTruncate( result, width );

    return result;
}

///////////////////////////////////////////////////////////////////////////////

string&
etAlignRight( const string& value, const string::size_type width, string& result )
{
    string::size_type len = etLength( value );

    result = "";

    if (len < width) {
        result.append( width-len, ' ' );
        result.append( value );
    }
    else if (len > width) {
        result = value;
        etTruncate( result, width );
    }
    else {
        result = value;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

string&
etDecolorize( string& value )
{
    string result;

    const string::size_type max = value.length();
    for ( string::size_type i = 0; i < max; i++ ) {
        const char c = value[i];
        switch (c) {
            case 27:
            case '^':
                if ((max-i) > 1)
                    i++;
                continue;

            default:
                result += c;
                break;
        }
    }

    value = result;
    return value;
}

///////////////////////////////////////////////////////////////////////////////

string::size_type
etLength( const string& value )
{
    string::size_type count = 0;
    bool incolor = false;

    const string::size_type max = value.length();
    for ( string::size_type i = 0; i < max; i++ ) {
        const char c = value[i];

        if (incolor) {
            switch (c) {
                case '^':
                    count++;
                    break;

                default:
                    incolor = false;
                    break;
            }
        }
        else {
            switch (c) {
                case '^':
                    incolor = true;
                    break;

                default:
                    count++;
                    break;
            }
        }
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

string
etTruncate( const string& value, const string::size_type len )
{
    string result = value;
    return etTruncate( result, len );
}

///////////////////////////////////////////////////////////////////////////////

string&
etTruncate( string& value, const string::size_type len )
{
    string::size_type count = 0;
    bool incolor = false;

    const string orig = value;
    value.clear();

    const string::size_type max = orig.length();
    for ( string::size_type i = 0; i < max && count < len; i++ ) {
        const char c = orig[i];

        if (incolor) {
            switch (c) {
                case '^':
                    count++;
                    break;

                default:
                    incolor = false;
                    break;
            }
        }
        else {
            switch (c) {
                case '^':
                    incolor = true;
                    break;

                default:
                    count++;
                    break;
            }
        }
        value += c;
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////////

string
toHexString( uint32 value )
{
    string result;
    return toHexString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toHexString( uint32 value, string& out )
{
    ostringstream oss;
    oss << "0x" << hex << setw(8) << setfill('0') << value;
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

int
toSeconds( const string& value, bool ms )
{
    int dfactor, hfactor, mfactor, sfactor;
    if (ms) {
        dfactor = 86400000;
        hfactor = 3600000;
        mfactor = 60000;
        sfactor = 1000;
    }
    else {
        dfactor = 86400;
        hfactor = 3600;
        mfactor = 60;
        sfactor = 1;
    }

    typedef enum {
        NONE,
        DAYS,
        HOURS,
        MINUTES,
        SECONDS,
    } progress_t;

    progress_t progress = NONE;

    int ivalue = 0;
    string field;

    const string::size_type max = value.length();
    for (string::size_type i = 0; i < max; i++) {
        const char c = value[i];
        switch (c) {
            case ' ':
            case '\t':
                if (field.length()) {
                    if (progress >= SECONDS)
                        return 0;
                    ivalue += atoi( field.c_str() ) * sfactor;
                    field.resize( 0 );
                    progress = SECONDS;
                }
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                field += c;
                break;

            case 'D':
            case 'd':
                if (!field.length() || progress >= DAYS)
                    return 0;
                ivalue += atoi( field.c_str() ) * dfactor;
                field.resize( 0 );
                progress = DAYS;
                break;

            case 'H':
            case 'h':
                if (!field.length() || progress >= HOURS)
                    return 0;
                ivalue += atoi( field.c_str() ) * hfactor;
                field.resize( 0 );
                progress = HOURS;
                break;

            case 'M':
            case 'm':
                if (!field.length() || progress >= MINUTES)
                    return 0;
                ivalue += atoi( field.c_str() ) * mfactor;
                field.resize( 0 );
                progress = MINUTES;
                break;

            case 'S':
            case 's':
                if (!field.length() || progress >= SECONDS)
                    return 0;
                ivalue += atoi( field.c_str() ) * sfactor;
                field.resize( 0 );
                progress = SECONDS;
                break;

            default:
                return 0;
        }
    }

    // Anything not claimed will be seconds
    if (field.length()) {
        if (progress >= SECONDS)
            return 0;
        ivalue += atoi( field.c_str() ) * sfactor;
        field.resize( 0 );
        progress = SECONDS;
    }

    return ivalue;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( const vec3_t& v )
{
    string result;
    return toString( v, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( const vec3_t& v, string& out )
{
    ostringstream oss;
    oss.flags( ios::fixed );
    oss.precision( 4 );
    oss << "[" << setw(10) << v[0] << " " << setw(10) << v[1] << " " << setw(10) << v[2] << "]";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( const void* ptr )
{
    string result;
    return toHexString( (uint32)ptr, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( const void* ptr, string& out )
{
    return toHexString( (uint32)ptr, out );
}

///////////////////////////////////////////////////////////////////////////////

string
toString( entityType_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( entityType_t type, string& out )
{
    switch (type) {
        case ET_AAGUN:                      out = "AAGUN";                      return out;
        case ET_AI_EFFECT:                  out = "AI_EFFECT";                  return out;
        case ET_ALARMBOX:                   out = "ALARMBOX";                   return out;
        case ET_ALIGNED_HIT_VOLUME:         out = "ALIGNED_HIT_VOLUME";         return out;
        case ET_ATTRACTOR_HINT:             out = "ATTRACTOR_HINT";             return out;
        case ET_BEAM:                       out = "BEAM";                       return out;
        case ET_BEAM_2:                     out = "BEAM_2";                     return out;
        case ET_BOMB:                       out = "BOMB";                       return out;
        case ET_BOTGOAL_INDICATOR:          out = "BOTGOAL_INDICATOR";          return out;
        case ET_BULLET_VOLUME:              out = "BULLET_VOLUME";              return out;
        case ET_CABINET_A:                  out = "CABINET_A";                  return out;
        case ET_CABINET_H:                  out = "CABINET_H";                  return out;
        case ET_CAMERA:                     out = "CAMERA";                     return out;
        case ET_COMMANDMAP_MARKER:          out = "COMMANDMAP_MARKER";          return out;
        case ET_CONCUSSIVE_TRIGGER:         out = "CONCUSSIVE_TRIGGER";         return out;
        case ET_CONSTRUCTIBLE:              out = "CONSTRUCTIBLE";              return out;
        case ET_CONSTRUCTIBLE_INDICATOR:    out = "CONSTRUCTIBLE_INDICATOR";    return out;
        case ET_CONSTRUCTIBLE_MARKER:       out = "CONSTRUCTIBLE_MARKER";       return out;
        case ET_CORONA:                     out = "CORONA";                     return out;
        case ET_CORPSE:                     out = "CORPSE";                     return out;
        case ET_EF_SPOTLIGHT:               out = "EF_SPOTLIGHT";               return out;
        case ET_EXPLOSIVE:                  out = "EXPLOSIVE";                  return out;
        case ET_EXPLOSIVE_INDICATOR:        out = "EXPLOSIVE_INDICATOR";        return out;
        case ET_EXPLO_PART:                 out = "EXPLO_PART";                 return out;
        case ET_FIRE_COLUMN:                out = "FIRE_COLUMN";                return out;
        case ET_FIRE_COLUMN_SMOKE:          out = "FIRE_COLUMN_SMOKE";          return out;
        case ET_FLAMEBARREL:                out = "FLAMEBARREL";                return out;
        case ET_FLAMETHROWER_CHUNK:         out = "FLAMETHROWER_CHUNK";         return out;
        case ET_FOOTLOCKER:                 out = "FOOTLOCKER";                 return out;
        case ET_FP_PARTS:                   out = "FP_PARTS";                   return out;
        case ET_GAMEMANAGER:                out = "GAMEMANAGER";                return out;
        case ET_GAMEMODEL:                  out = "GAMEMODEL";                  return out;
        case ET_GENERAL:                    out = "GENERAL";                    return out;
        case ET_HEALER:                     out = "HEALER";                     return out;
        case ET_INVISIBLE:                  out = "INVISIBLE";                  return out;
        case ET_ITEM:                       out = "ITEM";                       return out;
        case ET_LANDMINESPOT_HINT:          out = "LANDMINESPOT_HINT";          return out;
        case ET_LANDMINE_HINT:              out = "LANDMINE_HINT";              return out;
        case ET_MG42_BARREL:                out = "MG42_BARREL";                return out;
        case ET_MISSILE:                    out = "MISSILE";                    return out;
        case ET_MOVER:                      out = "MOVER";                      return out;
        case ET_MOVERSCALED:                out = "MOVERSCALED";                return out;
        case ET_OID_TRIGGER:                out = "OID_TRIGGER";                return out;
        case ET_ORIENTED_HIT_VOLUME:        out = "ORIENTED_HIT_VOLUME";        return out;
        case ET_PLAYER:                     out = "PLAYER";                     return out;
        case ET_PORTAL:                     out = "PORTAL";                     return out;
        case ET_PROP:                       out = "PROP";                       return out;
        case ET_PUSH_TRIGGER:               out = "PUSH_TRIGGER";               return out;
        case ET_RAMJET:                     out = "RAMJET";                     return out;
        case ET_SMOKER:                     out = "SMOKER";                     return out;
        case ET_SNIPER_HINT:                out = "SNIPER_HINT";                return out;
        case ET_SPEAKER:                    out = "SPEAKER";                    return out;
        case ET_SUPPLIER:                   out = "SUPPLIER";                   return out;
        case ET_TANK_INDICATOR:             out = "TANK_INDICATOR";             return out;
        case ET_TANK_INDICATOR_DEAD:        out = "TANK_INDICATOR_DEAD";        return out;
        case ET_TELEPORT_TRIGGER:           out = "TELEPORT_TRIGGER";           return out;
        case ET_TEMPHEAD:                   out = "TEMPHEAD";                   return out;
        case ET_TEMPLEGS:                   out = "TEMPLEGS";                   return out;
        case ET_TRAP:                       out = "TRAP";                       return out;
        case ET_TRIGGER_FLAGONLY:           out = "TRIGGER_FLAGONLY";           return out;
        case ET_TRIGGER_FLAGONLY_MULTIPLE:  out = "TRIGGER_FLAGONLY_MULTIPLE";  return out;
        case ET_TRIGGER_MULTIPLE:           out = "TRIGGER_MULTIPLE";           return out;
        case ET_WAYPOINT:                   out = "WAYPOINT";                   return out;
        case ET_WOLF_OBJECTIVE:             out = "WOLF_OBJECTIVE";             return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( gamestate_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( gamestate_t type, string& out )
{
    switch (type) {
        case GS_INITIALIZE:           out = "INITIALIZE";           return out;
        case GS_PLAYING:              out = "PLAYING";              return out;
        case GS_WARMUP_COUNTDOWN:     out = "WARMUP_COUNTDOWN";     return out;
        case GS_WARMUP:               out = "WARMUP";               return out;
        case GS_INTERMISSION:         out = "INTERMISSION";         return out;
        case GS_WAITING_FOR_PLAYERS:  out = "WAITING_FOR_PLAYERS";  return out;
        case GS_RESET:                out = "RESET";                return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( gametype_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( gametype_t type, string& out )
{
    switch (type) {
        case GT_WOLF:            out = "OBJECTIVE";  return out;
        case GT_WOLF_STOPWATCH:  out = "STOPWATCH";  return out;
        case GT_WOLF_CAMPAIGN:   out = "CAMPAIGN";   return out;
        case GT_WOLF_LMS:        out = "LMS";        return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( team_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( team_t type, string& out )
{
    switch (type) {
        case TEAM_FREE:       out = "FREE";       return out;
        case TEAM_AXIS:       out = "AXIS";       return out;
        case TEAM_ALLIES:     out = "ALLIES";     return out;
        case TEAM_SPECTATOR:  out = "SPECTATOR";  return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( trType_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( trType_t type, string& out )
{
    switch (type) {
        case TR_STATIONARY:        out = "STATIONARY";        return out;
        case TR_INTERPOLATE:       out = "INTERPOLATE";       return out;
        case TR_LINEAR:            out = "LINEAR";            return out;
        case TR_LINEAR_STOP:       out = "LINEAR_STOP";       return out;
        case TR_LINEAR_STOP_BACK:  out = "LINEAR_STOP_BACK";  return out;
        case TR_SINE:              out = "SINE";              return out;
        case TR_GRAVITY:           out = "GRAVITY";           return out;
        case TR_GRAVITY_LOW:       out = "GRAVITY_LOW";       return out;
        case TR_GRAVITY_FLOAT:     out = "GRAVITY_FLOAT";     return out;
        case TR_GRAVITY_PAUSED:    out = "GRAVITY_PAUSED";    return out;
        case TR_ACCELERATE:        out = "ACCELERATE";        return out;
        case TR_DECCELERATE:       out = "DECCELERATE";       return out;
        case TR_SPLINE:            out = "SPLINE";            return out;
        case TR_LINEAR_PATH:       out = "LINEAR_PATH";       return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toString( weapon_t type )
{
    string result;
    return toString( type, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toString( weapon_t type, string& out )
{
    switch (type) {
        case VERYBIGEXPLOSION:         out = "YBIGEXPLOSION";         return out;
        case WP_ADRENALINE_SHARE:      out = "ADRENALINE_SHARE";      return out;
        case WP_AKIMBO_COLT:           out = "AKIMBO_COLT";           return out;
        case WP_AKIMBO_LUGER:          out = "AKIMBO_LUGER";          return out;
        case WP_AKIMBO_SILENCEDCOLT:   out = "AKIMBO_SILENCEDCOLT";   return out;
        case WP_AKIMBO_SILENCEDLUGER:  out = "AKIMBO_SILENCEDLUGER";  return out;
        case WP_AMMO:                  out = "AMMO";                  return out;
        case WP_ARTY:                  out = "ARTY";                  return out;
        case WP_BINOCULARS:            out = "BINOCULARS";            return out;
        case WP_CARBINE:               out = "CARBINE";               return out;
        case WP_COLT:                  out = "COLT";                  return out;
        case WP_DUMMY_MG42:            out = "DUMMY_MG42";            return out;
        case WP_DYNAMITE:              out = "DYNAMITE";              return out;
        case WP_FG42:                  out = "FG42";                  return out;
        case WP_FG42SCOPE:             out = "FG42SCOPE";             return out;
        case WP_FLAMETHROWER:          out = "FLAMETHROWER";          return out;
        case WP_GARAND:                out = "GARAND";                return out;
        case WP_GARAND_SCOPE:          out = "GARAND_SCOPE";          return out;
        case WP_GPG40:                 out = "GPG40";                 return out;
        case WP_GRENADE_LAUNCHER:      out = "GRENADE_LAUNCHER";      return out;
        case WP_GRENADE_PINEAPPLE:     out = "GRENADE_PINEAPPLE";     return out;
        case WP_K43:                   out = "K43";                   return out;
        case WP_K43_SCOPE:             out = "K43_SCOPE";             return out;
        case WP_KAR98:                 out = "KAR98";                 return out;
        case WP_KNIFE:                 out = "KNIFE";                 return out;
        case WP_LANDMINE:              out = "LANDMINE";              return out;
        case WP_LANDMINE_BBETTY:       out = "LANDMINE_BBETTY";       return out;
        case WP_LANDMINE_PGAS:         out = "LANDMINE_PGAS";         return out;
        case WP_LOCKPICK:              out = "LOCKPICK";              return out;
        case WP_LUGER:                 out = "LUGER";                 return out;
        case WP_M7:                    out = "M7";                    return out;
        case WP_M97:                   out = "M97";                   return out;
        case WP_MAPMORTAR:             out = "MAPMORTAR";             return out;
        case WP_MEDIC_ADRENALINE:      out = "MEDIC_ADRENALINE";      return out;
        case WP_MEDIC_SYRINGE:         out = "MEDIC_SYRINGE";         return out;
        case WP_MEDKIT:                out = "MEDKIT";                return out;
        case WP_MOBILE_MG42:           out = "MOBILE_MG42";           return out;
        case WP_MOBILE_MG42_SET:       out = "MOBILE_MG42_SET";       return out;
        case WP_MORTAR:                out = "MORTAR";                return out;
        case WP_MORTAR_SET:            out = "MORTAR_SET";            return out;
        case WP_MP40:                  out = "MP40";                  return out;
        case WP_NONE:                  out = "NONE";                  return out;
        case WP_PANZERFAUST:           out = "PANZERFAUST";           return out;
        case WP_PLIERS:                out = "PLIERS";                return out;
        case WP_POISON_GAS:            out = "POISON_GAS";            return out;
        case WP_POISON_SYRINGE:        out = "POISON_SYRINGE";        return out;
        case WP_SATCHEL:               out = "SATCHEL";               return out;
        case WP_SATCHEL_DET:           out = "SATCHEL_DET";           return out;
        case WP_SILENCED_COLT:         out = "SILENCED_COLT";         return out;
        case WP_SILENCER:              out = "SILENCER";              return out;
        case WP_SMOKETRAIL:            out = "SMOKETRAIL";            return out;
        case WP_SMOKE_BOMB:            out = "SMOKE_BOMB";            return out;
        case WP_SMOKE_MARKER:          out = "SMOKE_MARKER";          return out;
        case WP_STEN:                  out = "STEN";                  return out;
        case WP_THOMPSON:              out = "THOMPSON";              return out;
        case WP_TRIPMINE:              out = "TRIPMINE";              return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << type << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
toStringSecondsRemaining( int seconds, bool relative )
{
    string result;
    return toStringSecondsRemaining( seconds, result, relative );
}

///////////////////////////////////////////////////////////////////////////////

string&
toStringSecondsRemaining( int seconds, string& out, bool relative )
{
    int secs = relative ? seconds : (seconds - time( NULL ));
    int days = secs / (60*60*24);
    secs -= (days * (60*60*24));
    int hours = secs / (60*60);
    secs -= (hours * (60*60));
    int mins = secs / 60;
    secs -= (mins * 60);

    bool append = false;
    ostringstream oss;

    if (days) {
        if (append)
            oss << ", ";
        append = true;
        oss << days << " day";
        if (days != 1)
            oss << 's';
    }

    if (hours) {
        if (append)
            oss << ", ";
        append = true;
        oss << hours << " hour";
        if (hours != 1)
            oss << 's';
    }

    if (mins) {
        if (append)
            oss << ", ";
        append = true;
        oss << mins << " minute";
        if (mins != 1)
            oss << 's';
    }

    if (!append || secs) {
        if (append)
            oss << ", ";
        append = true;
        oss << secs << " second";
        if (secs != 1)
            oss << 's';
    }

    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
