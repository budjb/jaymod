#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

MuzzleBulletModel::MuzzleBulletModel( Client& client_, bool reference_ )
    : AbstractBulletModel ( TYPE_MUZZLE, client_, reference_ )
    , _bulletVol          ( *this, true )
{
}

///////////////////////////////////////////////////////////////////////////////

MuzzleBulletModel::~MuzzleBulletModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
MuzzleBulletModel::adjustStartPoint( vec3_t start )
{
    // We currently do not support unmobile MG42 adjustments (eg: tank-mounted browning or crew-gun nest mg42)
    // ( future data: browning (tank) offsets: 38.421875 -0.265625 -2.515625 )
    if (client.gentity.s.eFlags & EF_MG42_ACTIVE)
        return;

    // Fetch origins/orientations for player model.
    grefEntity_t re;
    // Mike TODO: we might want want snapshot time here.
    mdx_gentity_to_grefEntity( &client.gentity, &re, level.time );

    vec3_t        origins[2];  // 0=right, 1=left
    orientation_t orients[2];  // 0=right, 1=left
    mdx_weapon_positions( client.gentity, re, origins, orients );

    // Apply world axis to local axis we need for this function body.
    {
        vec3_t axis[3];

        MatrixMultiply( re.torsoAxis, orients[0].axis, axis );
        MatrixMultiply( axis, re.axis, orients[0].axis );

        MatrixMultiply( re.torsoAxis, orients[1].axis, axis );
        MatrixMultiply( axis, re.axis, orients[1].axis );
    }

    vec3_t muzzleOffset;
    switch (client.gentity.s.weapon) {
        case WP_AKIMBO_COLT:
        case WP_AKIMBO_LUGER:
        case WP_COLT:
        case WP_LUGER:
            VectorSet( muzzleOffset, 10.453125f, 0.234375f, 0.375000f );
            break;

        case WP_AKIMBO_SILENCEDCOLT:
        case WP_AKIMBO_SILENCEDLUGER:
        case WP_SILENCED_COLT:
        case WP_SILENCER:
            VectorSet( muzzleOffset, 17.156250f, 0.000000f, 1.484375f );
            break;

        case WP_FG42:
        case WP_FG42SCOPE:
            VectorSet( muzzleOffset, 28.515625f, 0.046875f, 3.109375f );
            break;

        case WP_GARAND:
        case WP_GARAND_SCOPE:
            VectorSet( muzzleOffset, 39.578125f, 0.000000f, 1.359375f );
            break;

        case WP_K43:
        case WP_K43_SCOPE:
            VectorSet( muzzleOffset, 37.703125f, 0.000000f, 1.359375f );
            break;

        case WP_MOBILE_MG42:
        case WP_MOBILE_MG42_SET:
            VectorSet( muzzleOffset, 41.906000f, 0.489000f, 1.846000f );
            break;

        case WP_M97:      VectorSet( muzzleOffset, 29.517412f, 0.086034f, 3.737498f ); break;
        case WP_MP40:     VectorSet( muzzleOffset, 23.375000f, 0.234375f, 2.843750f ); break;
        case WP_THOMPSON: VectorSet( muzzleOffset, 20.078125f, 0.234375f, 3.390625f ); break;
        case WP_STEN:     VectorSet( muzzleOffset, 19.734375f, 1.156250f, 1.843750f ); break;

        default:
            // If we do not explicitly support weapon then do nothing for default ET-SDK behavior.
            return;
    }

    // Our starting point will be the right-hand.
    int hand = 0;

    // Check if we are firing from left-hand on weapons capable of it.
    switch (client.gentity.s.weapon) {
        case WP_AKIMBO_COLT:
        case WP_AKIMBO_LUGER:
        case WP_AKIMBO_SILENCEDCOLT:
        case WP_AKIMBO_SILENCEDLUGER:
            if ( BG_AkimboFireSequence(
                     client.gclient.ps.weapon,
                     client.gclient.ps.ammoclip[ BG_FindClipForWeapon( weapon_t(client.gclient.ps.weapon) )],
                     client.gclient.ps.ammoclip[ BG_FindClipForWeapon( weapon_t(BG_AkimboSidearm( client.gclient.ps.weapon )))] ))
            {
                hand = 1;
            }

            break;

        default:
            break;
    }

    VectorCopy( origins[hand], start );

    // Since game-engine does not expose tags we hard-code this for now.
    // Yes it's ugly, but I don't have time to put md3/mdc parser into qagame.

    if (muzzleOffset[0] != 0)
        VectorMA( start, muzzleOffset[0], orients[hand].axis[0], start );

    if (muzzleOffset[1] != 1)
        VectorMA( start, muzzleOffset[1], orients[hand].axis[1], start );

    if (muzzleOffset[2] != 2)
        VectorMA( start, muzzleOffset[2], orients[hand].axis[2], start );
}
