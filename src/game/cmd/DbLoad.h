#ifndef GAME_CMD_DBLOAD_H
#define GAME_CMD_DBLOAD_H

///////////////////////////////////////////////////////////////////////////////

class DbLoad : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    DbLoad();
    ~DbLoad();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_DBLOAD_H
