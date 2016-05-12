#ifndef GAME_CMD_CANCELVOTE_H
#define GAME_CMD_CANCELVOTE_H

///////////////////////////////////////////////////////////////////////////////

class CancelVote : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    CancelVote();
    ~CancelVote();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_CANCELVOTE_H
