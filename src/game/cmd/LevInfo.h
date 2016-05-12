#ifndef GAME_CMD_SHOWLEVEL_H
#define GAME_CMD_SHOWLEVEL_H

///////////////////////////////////////////////////////////////////////////////

class LevInfo : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    LevInfo();
    ~LevInfo();

public:
    static void doPrivileges( Buffer&, const PrivilegeSet&, const PrivilegeSet&, InlineText&, InlineText&, const string& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_SHOWLEVEL_H
