#ifndef GAME_DATABASE_H
#define GAME_DATABASE_H

///////////////////////////////////////////////////////////////////////////////

/*
 * Database class is the base implementation for persistent databases.
 *
 * All databases use an ASCII file format:
 *
 *     1. Leading/trailing spaces on any line are trimmed.
 *     2. Lines beginning with '#' are considered comments.
 *     3. Lines of { <NAME><DELIMITER><VALUE><EOL> } form NAME/VALUEs pairs.
 *     4. <NAME> cannot contain any <DELIMITER> characters.
 *     5. <DELIMITER> is any combination of { ' ', '=', '\t' }.
 *     6. <VALUE> continues until <EOL>.
 *     7. <EOL> is optional for last line.
 *     8. <NAME> of value this._key marks the beginning of a new record.
 *     9. <NAME> is case-insensitive.
 *    10. Subsequent lines of NAME/VALUE pairs are part of the same record.
 *
 */
class Database {
public:
    virtual ~Database();

protected:
    /**************************************************************************
     * Constructor.
     *
     * Param file specifies filename for IO operations.
     * Param key specifies the name of key for record boundries.
     *
     */
    Database( const string& file, const string& key );

    /**************************************************************************
     * Close database file.
     *
     */
    void close();

    /**************************************************************************
     * Open database file.
     *
     * Param write true to open file for writing, otherwise for reading.
     *
     * Returns true if error ocurred.
     *
     */
    bool open( bool write, string& );

    /**************************************************************************
     * Parse a record-data from open intput stream.
     *
     * Param data is populated with NAME/VALUE pairs if any.
     *
     */
    void parseData( map<string,string>& data );

    /*************************************************************************/

    void logBegin ( bool, const string& );
    void logEnd   ( int, const string& );

    /*************************************************************************/

    const string  _filename; // filename (basename only) used for database
    const string  _key;      // name of key for record boundries
    fstream       _stream;   // IO object

private:
    /**************************************************************************
     * Parse a name/value pair from open input stream.
     *
     * Param name stores the parsed name.
     * Param value stores the parsed value.
     *
     * Returns true if no name/value pair was parsed.
     *
     */
    bool parsePair( string& name, string& value );

    /*************************************************************************/

    string _currentKeyValue;
    Process::mstime_t _logbeginTimestamp;
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_DATABASE_H
