#ifndef GAME_CMD_ABSTRACTBUILTIN_H
#define GAME_CMD_ABSTRACTBUILTIN_H

///////////////////////////////////////////////////////////////////////////////

class AbstractBuiltin : public AbstractCommand
{
public:
    ~AbstractBuiltin();

protected:
    AbstractBuiltin( const char*, bool = false );

private:
    AbstractBuiltin();  // not permitted
};

///////////////////////////////////////////////////////////////////////////////

#include <game/cmd/About.h>
#include <game/cmd/AdminTest.h>
#include <game/cmd/Ban.h>
#include <game/cmd/BanInfo.h>
#include <game/cmd/BanList.h>
#include <game/cmd/BanUser.h>
#include <game/cmd/CancelVote.h>
#include <game/cmd/Chicken.h>
#include <game/cmd/CrazyGravity.h>
#include <game/cmd/CryBaby.h>
#include <game/cmd/DbLoad.h>
#include <game/cmd/DbSave.h>
#include <game/cmd/Disorient.h>
#include <game/cmd/FTime.h>
#include <game/cmd/Finger.h>
#include <game/cmd/Fling.h>
#include <game/cmd/Flinga.h>
#include <game/cmd/Glow.h>
#include <game/cmd/Help.h>
#include <game/cmd/Kick.h>
#include <game/cmd/Launch.h>
#include <game/cmd/Launcha.h>
#include <game/cmd/LevAdd.h>
#include <game/cmd/LevDelete.h>
#include <game/cmd/LevEdit.h>
#include <game/cmd/LevInfo.h>
#include <game/cmd/LevList.h>
#include <game/cmd/ListPlayers.h>
#include <game/cmd/Lock.h>
#include <game/cmd/Lol.h>
#include <game/cmd/LsPlayers.h>
#include <game/cmd/Mute.h>
#include <game/cmd/News.h>
#include <game/cmd/NextMap.h>
#include <game/cmd/Orient.h>
#include <game/cmd/Page.h>
#include <game/cmd/Pants.h>
#include <game/cmd/PanzerWar.h>
#include <game/cmd/Pause.h>
#include <game/cmd/Pip.h>
#include <game/cmd/Pop.h>
#include <game/cmd/PutTeam.h>
#include <game/cmd/Rename.h>
#include <game/cmd/Reset.h>
#include <game/cmd/ResetXp.h>
#include <game/cmd/ResetmyXp.h>
#include <game/cmd/Restart.h>
#include <game/cmd/Revive.h>
#include <game/cmd/Seen.h>
#include <game/cmd/SetLevel.h>
#include <game/cmd/Shake.h>
#include <game/cmd/Shuffle.h>
#include <game/cmd/Slap.h>
#include <game/cmd/Smite.h>
#include <game/cmd/SniperWar.h>
#include <game/cmd/Spec.h>
#include <game/cmd/Spec999.h>
#include <game/cmd/Splat.h>
#include <game/cmd/Splata.h>
#include <game/cmd/Status.h>
#include <game/cmd/Swap.h>
#include <game/cmd/Throw.h>
#include <game/cmd/Throwa.h>
#include <game/cmd/Time.h>
#include <game/cmd/Unban.h>
#include <game/cmd/Unlock.h>
#include <game/cmd/Unmute.h>
#include <game/cmd/Unpause.h>
#include <game/cmd/Uptime.h>
#include <game/cmd/UserDelete.h>
#include <game/cmd/UserEdit.h>
#include <game/cmd/UserInfo.h>
#include <game/cmd/UserList.h>

///////////////////////////////////////////////////////////////////////////////

namespace builtins {
    extern About        about;
    extern AdminTest    adminTest;
    extern Ban          ban;
    extern BanInfo      banInfo;
    extern BanList      banList;
    extern BanUser      banUser;
    extern CancelVote   cancelVote;
    extern CrazyGravity crazygravity;
    extern CryBaby      crybaby;
    extern DbLoad       dbLoad;
    extern DbSave       dbSave;
    extern Disorient    disorient;
    extern FTime        ftime;
    extern Finger       finger;
    extern Fling        fling;
    extern Flinga       flinga;
    extern Glow         glow;
    extern Help         help;
    extern Kick         kick;
    extern Launch       launch;
    extern Launcha      launcha;
    extern LevAdd       levAdd;
    extern LevDelete    levDelete;
    extern LevEdit      levEdit;
    extern LevInfo      levInfo;
    extern LevList      levList;
    extern ListPlayers  listPlayers;
    extern Lock         lock;
    extern Lol          lol;
    extern LsPlayers    lsPlayers;
    extern Mute         mute;
    extern News         news;
    extern NextMap      nextMap;
    extern Orient       orient;
    extern Page         page;
    extern Pants        pants;
    extern PanzerWar    panzerwar;
    extern Pause        pause;
    extern Pip          pip;
    extern Pop          pop;;
    extern PutTeam      putTeam;
    extern Rename       rename;
    extern Reset        reset;
    extern ResetXp      resetXp;
    extern ResetmyXp    resetmyXp;
    extern Restart      restart;
    extern Revive       revive;
    extern Seen         seen;
    extern SetLevel     setLevel;
    extern Shake        shake;
    extern Shuffle      shuffle;
    extern Slap         slap;
    extern Smite        smite;
    extern SniperWar    sniperwar;
    extern Spec         spec;
    extern Spec999      spec999;
    extern Splat        splat;
    extern Splata       splata;
    extern Status       status;
    extern Swap         swap;
    extern Throw        throw_;
    extern Throwa       throwa;
    extern Time         time;
    extern Unban        unban;
    extern Unlock       unlock;
    extern Unmute       unmute;
    extern Unpause      unpause;
    extern Uptime       uptime;
    extern UserDelete   userDelete;
    extern UserEdit     userEdit;
    extern UserInfo     userInfo;
    extern UserList     userList;
} // namespace builtins

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_ABSTRACTBUILTIN_H
