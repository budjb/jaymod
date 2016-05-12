models/multiplayer/molotov/icon
{
    nomipmaps
    nopicmip
    {
        map models/multiplayer/molotov/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

models/multiplayer/molotov/molotov
{
    cull disable
    {
        map models/multiplayer/molotov/fluid.tga
        blendfunc blend
        rgbgen lightingdiffuse
        tcmod scale 4 6
        tcmod scroll 0 .8
    }
    {
        map models/multiplayer/molotov/molotov.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        rgbgen lightingdiffuse
    }
}
