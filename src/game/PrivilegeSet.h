#ifndef GAME_PRIVILEGESET_H
#define GAME_PRIVILEGESET_H

///////////////////////////////////////////////////////////////////////////////

class PrivilegeSet
{
public:
    typedef set<Privilege::Handle>        HandleSet;
    typedef map<string,Privilege::Handle> IndexName;

private:
    HandleSet __handleSet;  // sorted by handle
    IndexName __indexName;  // map lower-case name -> handle

public:
    PrivilegeSet();
    ~PrivilegeSet();

    PrivilegeSet& operator=( const PrivilegeSet& );

    void clear    ( );
    bool contains ( const Privilege& ) const;
    bool empty    ( ) const;
    void erase    ( const Privilege& );
    void erase    ( const list<const Privilege*>& );
    void insert   ( const Privilege& );

    const HandleSet& _handleSet;  // sorted by handle
    const IndexName& _indexName;  // map lower-case name -> handle

public:
    static void canonicalize( PrivilegeSet&, PrivilegeSet& );

    static void encode( const PrivilegeSet&, const PrivilegeSet&, ostream& );
    static void encode( const PrivilegeSet*, const PrivilegeSet*, ostream& );
    static void decode( PrivilegeSet&, PrivilegeSet&, const string&, const string& );

    static PrivilegeSet REGISTRY;
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_PRIVILEGE_H
