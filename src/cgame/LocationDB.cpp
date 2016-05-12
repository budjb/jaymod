#include <bgame/impl.h>

LocationDB::LocationDB()
    : line   ( 1 )
    , loaded ( false )
{
}

LocationDB::~LocationDB()
{
}

bool LocationDB::isLoaded()
{
    return loaded;
}

bool LocationDB::load()
{
	stringstream buffer;

	if (!open(buffer))
		return false;

    int ret;
    while ((ret = readLocation(buffer)) > 0);

    if (ret < 0) {
        clean();
        return false;
    }

    loaded = true;

	return true;
}

void LocationDB::clean()
{
    for (list_t::iterator i = locationList.begin(); i != locationList.end(); i++) {
        delete *i;
    }

    locationList.clear();
}

bool LocationDB::open( stringstream& buffer )
{
	int				len;
	fileHandle_t	file;

	// Try the override first
	len = trap_FS_FOpenFile(va("maps/%s_loc_override.dat", cgs.rawmapname), &file, FS_READ);

	// If it wasn't found, try opening the default
	if (len < 0) {
		len = trap_FS_FOpenFile(va("maps/%s_loc.dat", cgs.rawmapname), &file, FS_READ);

		if (len < 0) {
			return false;
		}
	}

	// Read in the file and place into the stringstream
    char* tmpbuf = (char*)malloc(len + 1);
	trap_FS_Read(tmpbuf, len, file);					// read the file into the buffer
	tmpbuf[len] = 0;								// make sure it's null-terminated
	trap_FS_FCloseFile( file );                         // close the file, we're done with it
	buffer.str(tmpbuf);
    free(tmpbuf);

	return true;
}

int LocationDB::readLocation ( stringstream& buffer )
{
    int ret;
    int x, y, z;
    string msg;

    string value;

    // Read the X coord
    if ((ret = readInt(buffer, value)) <= 0)
        return ret;
    x = atoi(value.c_str());

    // Read the Y coord
    if ((ret = readInt(buffer, value)) <= 0)
        return -1;
    y = atoi(value.c_str());

    // Read the Z coord
    if ((ret = readInt(buffer, value)) <= 0)
        return -1;
    z = atoi(value.c_str());

    // Read the location description
    if ((ret = readString(buffer, value)) <= 0)
        return -1;
    msg = value;

    if (msg == "@") {
        if (currentDesc.length() == 0)
            return -1;
        else
            msg = currentDesc;
    } else {
        currentDesc = msg;
    }

    // Print it out
    Location* location = new Location(x, y, z, msg);

    locationList.push_back(location);

    return 1;
}

int LocationDB::readInt( stringstream& buffer, string& value )
{
    int ret = strip(buffer);

    if (ret <= 0)
        return ret;

    value = "";

    for (char c; !buffer.get(c).rdstate(); ) {
        // If we get a whitespace char, put it back and return
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '/') {
            buffer.unget();
            return 1;
        }

        // If it's not one of these, BAD!
        if (c != '-' && (c < '0' || c > '9'))
            return -1;

        value += c;
    }

	return 1;
}

int LocationDB::readString( stringstream& buffer, string& value )
{
    int ret = strip(buffer);

    if (ret <= 0)
        return ret;

    value = "";

    for (char c; !buffer.get(c).rdstate(); ) {
        // If we get a newline, put it back and return
        if (c == '\n' || c == '\r') {
            buffer.unget();
            return 1;
        }

        // If we get a ", just ignore it
        if (c == '"')
            continue;

        value += c;
    }

    return 1;
}

int LocationDB::strip( stringstream& buffer )
{
	enum mode_t {WHITESPACE, COMMENT_BEGIN, COMMENT_LINE, COMMENT_C, COMMENT_C_END};

	mode_t mode = WHITESPACE;

	for (char c; !buffer.get(c).rdstate(); ) {
		switch (mode) {
		case WHITESPACE:
			switch (c) {
			case '\n':
                line++;
                break;

			case ' ':
			case '\t':
			case '\r':
				break;

			case '/':
				mode = COMMENT_BEGIN;
				break;

			default:
				buffer.unget();
				return 1;
			}
			break;

		case COMMENT_BEGIN:
			switch (c) {
			case '/':
				mode = COMMENT_LINE;
				break;

			case '*':
				mode = COMMENT_C;
				break;

			default:
				return -1;
			}
			break;

		case COMMENT_LINE:
            switch (c) {
            case '\n':
                mode = WHITESPACE;
                line++;
                break;

            default:
                break;
            }
            break;

        case COMMENT_C:
            switch (c) {
            case '*':
                mode = COMMENT_C_END;
                break;

            case '\n':
                line++;
                break;
            
            default:
                break;
            }
            break;

        case COMMENT_C_END:
            switch (c) {
            case '/':
                mode = WHITESPACE;
                break;

            default:
                return -1;
            }
            break;
        }
	}

    return 0;
}

bool LocationDB::getLocation( vec3_t origin, string& desc )
{
    Location* best = NULL;
    float     bestdist	= -1;

    float     len;
    vec3_t    lenVec;

    // Just return false here as a convenience if it wasn't loaded
    if (!isLoaded())
        return false;

    for (list_t::const_iterator i = locationList.begin(); i != locationList.end(); i++) {
        Location* loc = *i;

        VectorSubtract( origin,loc->origin, lenVec );
        len = VectorLength( lenVec );

        if (!(cg_locationMode.integer & FTLOC_NOPVS))
            if (!trap_R_inPVS( origin, loc->origin ))
                continue;

        if ( bestdist != -1 && len > bestdist )
            continue;

        bestdist = len;
        best  = loc;
    }

    if (best) {
        desc = best->description;
        return true;
    }

    desc = "Unknown";
    return false;
} 

LocationDB locationDB;
