#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::args( vector<string>& out, const string& in )
{
    // Skip delimiters at beginning.
    string::size_type lastPos = in.find_first_not_of(' ', 0);

    // Find first "non-delimiter".
    string::size_type pos     = in.find_first_of(' ', lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        out.push_back(in.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = in.find_first_not_of(' ', pos);

        // Find next "non-delimiter"
        pos = in.find_first_of(' ', lastPos);
    }

    return out.size();
}

///////////////////////////////////////////////////////////////////////////////

Engine::Ptr Engine::ptr = (Engine::Ptr)-1;
