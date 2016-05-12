#ifndef GAME_CMD_CRAZYGRAVITY_H
#define GAME_CMD_CRAZYGRAVITY_H

///////////////////////////////////////////////////////////////////////////////

class CrazyGravity : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    CrazyGravity();
    ~CrazyGravity();

public:
    static void run();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_CRAZYGRAVITY_H
