#ifndef GAME_CMD_LISTPLAYERS_H
#define GAME_CMD_LISTPLAYERS_H

///////////////////////////////////////////////////////////////////////////////

class ListPlayers : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    ListPlayers();
    ~ListPlayers();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_LISTPLAYERS_H
