#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

void
CHitVolume::render( centity_t& ent, bool oriented )
{
    entityState_t& es = ent.currentState;

    if ( !(es.eFlags & HVF_ENABLED) || !(es.eFlags & HVF_DRAW) )
        return;

    const bool principal = es.modelindex &  0x80;
    const int  hvitality = es.modelindex & ~0x80;

    // Do not draw box in 1st-person (unless ghost).
    if (!cg.renderingThirdPerson && es.clientNum == cg.snap->ps.clientNum && hvitality != 3)
        return;

    // Setup refent.
    refEntity_t re;
    memset( &re, 0, sizeof(re) );

    re.reType            = RT_MODEL;
    re.renderfx          = RF_NOSHADOW;
    re.hModel            = oriented ? cgs.media.polygonCubeCO : cgs.media.polygonCubeOO;
    re.nonNormalizedAxes = qtrue;

    re.customShader = principal ? cgs.media.hitVolumeShader : cgs.media.hitVolumeGroupShader;

    switch (hvitality) {
        case 1: // PRINCIPAL --> CYAN
            {
                // Compute flash/color code.
                if (ent.muzzleFlashTime != ent.currentState.effect1Time) {
                    ent.muzzleFlashTime = ent.currentState.effect1Time;
                    ent.overheatTime = cg.time + 1500;
                }

                const int flashRemain = ent.overheatTime - cg.time;
                bool flash;
                if (flashRemain > 0)
                    flash = (flashRemain - 1) / 50 % 4;
                else
                    flash = false;

                if (flash) {
                    // YELLOW + opaqueness + alternate shader
                    re.shaderRGBA[0] = byte(1.0f * 255.0f);
                    re.shaderRGBA[1] = byte(1.0f * 255.0f);
                    re.shaderRGBA[2] = byte(0.0f * 255.0f);
                    re.shaderRGBA[3] = byte(0.8f * 255.0f);

                    re.customShader = cgs.media.hitVolumeHitShader;
                }
                else {
                    // CYAN
                    re.shaderRGBA[0] = byte(0.0f * 255.0f);
                    re.shaderRGBA[1] = byte(1.0f * 255.0f);
                    re.shaderRGBA[2] = byte(1.0f * 255.0f);
                    re.shaderRGBA[3] = byte(0.6f * 255.0f);
                }
            }
            break;

        case 2: // REFERENCE -> RED
            re.shaderRGBA[0] = byte(1.0f * 255.0f);
            re.shaderRGBA[1] = byte(0.0f * 255.0f);
            re.shaderRGBA[2] = byte(0.0f * 255.0f);
            re.shaderRGBA[3] = byte(0.4f * 255.0f);
            break;

        case 3: // GHOST -> ( WHITE / RED )
            if (es.eFlags & HVF_HIT) {
                re.shaderRGBA[0] = byte(1.0f * 255.0f);
                re.shaderRGBA[1] = byte(0.0f * 255.0f);
                re.shaderRGBA[2] = byte(0.0f * 255.0f);
                re.shaderRGBA[3] = byte(0.4f * 255.0f);

                re.customShader = cgs.media.hitVolumeHitShader;
            }
            else {
                re.shaderRGBA[0] = byte(1.0f * 255.0f);
                re.shaderRGBA[1] = byte(1.0f * 255.0f);
                re.shaderRGBA[2] = byte(1.0f * 255.0f);
                re.shaderRGBA[3] = byte(0.4f * 255.0f);
            }
            break;

        default: // UNKNOWN -> WHITE
            re.shaderRGBA[0] = byte(1.0f * 255.0f);
            re.shaderRGBA[1] = byte(1.0f * 255.0f);
            re.shaderRGBA[2] = byte(1.0f * 255.0f);
            re.shaderRGBA[3] = byte(0.5f * 255.0f);
            break;
    }

    AxisClear( re.axis );

    if (oriented) {
        // Apply entity rotation.
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
    }
    else {
        // Apply scaling matrix.
        re.axis[0][0] *= es.origin2[0];
        re.axis[1][1] *= es.origin2[1];
        re.axis[2][2] *= es.origin2[2];
    }

    // Set origins.
    VectorCopy( es.origin, re.origin ); 
    VectorCopy( es.origin, re.oldorigin );
    VectorCopy( es.origin, re.lightingOrigin );

    trap_R_AddRefEntityToScene( &re );
}
