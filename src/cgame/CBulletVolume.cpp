#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

void
CBulletVolume::render( centity_t& ent )
{
    entityState_t& es = ent.currentState;

    if (!(es.groundEntityNum & BVF_ENABLED))
        return;

    // Setup refent.
    refEntity_t re;
    memset( &re, 0, sizeof(re) );

    re.reType            = RT_MODEL;
    re.renderfx          = RF_NOSHADOW;
    re.hModel            = cgs.media.polygonCubeFO;
    re.nonNormalizedAxes = qtrue;
    re.customShader      = cgs.media.bulletVolumeShader;

    re.shaderRGBA[3] = byte(es.angles2[2] * 255.0f);
    switch (es.modelindex) {
        default: // UNKNWON -> GRAY
        case 0:
            re.shaderRGBA[0] = byte(0.5f * 255.0f);
            re.shaderRGBA[1] = byte(0.5f * 255.0f);
            re.shaderRGBA[2] = byte(0.5f * 255.0f);
            break;

        case 1: // NOHIT -> BLUE
            re.shaderRGBA[0] = byte(0.0f * 255.0f);
            re.shaderRGBA[1] = byte(0.0f * 255.0f);
            re.shaderRGBA[2] = byte(1.0f * 255.0f);
            break;

        case 2: // HIT -> RED
            re.shaderRGBA[0] = byte(1.0f * 255.0f);
            re.shaderRGBA[1] = byte(0.0f * 255.0f);
            re.shaderRGBA[2] = byte(0.0f * 255.0f);
            break;

        case 3: // REFERENCE NOHIT -> GREEN
            re.shaderRGBA[0] = byte(0.0f * 255.0f);
            re.shaderRGBA[1] = byte(1.0f * 255.0f);
            re.shaderRGBA[2] = byte(0.0f * 255.0f);
            break;

        case 4: // REFERENCE HIT -> YELLOW
            re.shaderRGBA[0] = byte(1.0f * 255.0f);
            re.shaderRGBA[1] = byte(1.0f * 255.0f);
            re.shaderRGBA[2] = byte(0.0f * 255.0f);
            break;
    }

    // Apply entity rotation.
    AxisClear( re.axis );
    AnglesToAxis( es.angles, re.axis );

    // Apply scaling matrix.
    vec3_t smatrix[3];
    AxisClear( smatrix );

    smatrix[0][0] *= es.origin2[0];
    smatrix[1][1] *= es.origin2[1];
    smatrix[2][2] *= es.origin2[2];

    vec3_t tmp[3];
    MatrixMultiply( smatrix, re.axis, tmp );
    AxisCopy( tmp, re.axis );

    // Set origins.
    VectorCopy( es.origin, re.origin );
    VectorCopy( es.origin, re.oldorigin );
    VectorCopy( es.origin, re.lightingOrigin );

    trap_R_AddRefEntityToScene( &re );
}
