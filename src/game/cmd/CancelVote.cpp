#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

CancelVote::CancelVote()
    : AbstractBuiltin( "cancelvote" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Voids the current vote.";
}

///////////////////////////////////////////////////////////////////////////////

CancelVote::~CancelVote()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
CancelVote::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    if (!level.voteInfo.voteTime || level.voteInfo.vote_fn == NULL || level.time - level.voteInfo.voteTime < 1000) {
        txt._ebuf << "There is no vote in progress.";
        return PA_ERROR;
    }

    level.voteInfo.voteNo = level.numConnectedClients;
    CheckVote();

    Buffer buf;
    buf << _name << ": " << xcvalue << "The current vote has been canceled.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
