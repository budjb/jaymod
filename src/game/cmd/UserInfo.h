#ifndef GAME_CMD_SHOWUSER_H
#define GAME_CMD_SHOWUSER_H

///////////////////////////////////////////////////////////////////////////////

class UserInfo : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    UserInfo();
    ~UserInfo();

public:
    static void doUser( Buffer&, const User&, InlineText&, InlineText& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_SHOWUSER_H
