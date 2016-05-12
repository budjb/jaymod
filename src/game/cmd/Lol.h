#ifndef GAME_CMD_LOL_H
#define GAME_CMD_LOL_H

///////////////////////////////////////////////////////////////////////////////

class Lol : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    Lol();
    ~Lol();

private:
    void run( Client&, int );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_LOL_H
