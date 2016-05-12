// shader name
watermark/jaymod
{
	nocompress
	nomipmaps
	nopicmip
	{
		// image filename
		map watermark/jaymod.tga
		blendFunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

sprites/net
{
	nocompress
	nopicmip
	{
		map sprites/net.tga
		rgbgen vertex
	}
}

models/multiplayer/syringe/poison
{
        cull disable
        {
                map models/multiplayer/poison/fluid.tga
                blendfunc blend
                rgbgen lightingdiffuse
                tcmod scale 4 6
                tcmod scroll 0 -.8
        }
        {
                map models/multiplayer/syringe/syringe.tga
                blendfunc gl_src_alpha gl_one_minus_src_alpha
                rgbgen lightingdiffuse
        }
        {
                map models/multiplayer/syringe/syringe_reflections.tga
                blendfunc gl_src_alpha gl_one_minus_src_alpha
                rgbgen lightingdiffuse
        }
}

models/multiplayer/syringe/adrenaline_share
{
        cull disable
        {
                map models/multiplayer/adrenaline_share/fluid.tga
                blendfunc blend
                rgbgen lightingdiffuse
                tcmod scale 4 6
                tcmod scroll 0 -.8
        }
        {
                map models/multiplayer/syringe/syringe.tga
                blendfunc gl_src_alpha gl_one_minus_src_alpha
                rgbgen lightingdiffuse
        }
        {
                map models/multiplayer/syringe/syringe_reflections.tga
                blendfunc gl_src_alpha gl_one_minus_src_alpha
                rgbgen lightingdiffuse
        }
}

sprites/poisoned
{
	nocompress
	nopicmip
	{
		map sprites/poisoned.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

etpro/color_construction
{
	cull none
	deformVertexes wave 1 sin -0.5 0 0 1
	noPicmip
	surfaceparm trans
	{
		map textures/sfx/construction.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen entity
		tcGen environment
		tcMod scroll 0.025 -0.07625
	}
}

ui/assets/icon_jaymod
{
	nomipmaps
	nopicmip
	{
		clampmap ui/assets/icon_jaymod.tga
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

ui/assets/icon_etpro
{
	nomipmaps
	nopicmip
	{
		clampmap ui/assets/icon_etpro.tga
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/2d/friendlycross
{
	nocompress
	nopicmip
	{
		map gfx/2d/friendlycross.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_m97_1_select
{
	nomipmaps
	nopicmip
	{
		map icons/iconw_m97_1_select.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

ui/assets/jaymod
{
    nomipmaps
    nopicmip
    {
        clampmap ui/assets/jaymod.tga
        blendfunc blend
        rgbGen vertex
        alphaGen vertex
    }
}

models/multiplayer/poison_gas/allies
{
    cull disable
    {
        map textures/effects/envmap_slate_90.tga
        rgbGen lightingdiffuse
        tcGen environment
    }
    {
        map models/multiplayer/poison_gas/cannister_allies.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
    }
}

models/multiplayer/poison_gas/axis
{
    cull disable
    {
        map textures/effects/envmap_slate_90.tga
        rgbGen lightingdiffuse
        tcGen environment
    }
    {
        map models/multiplayer/poison_gas/cannister_axis.tga
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_bbetty/landmine
{
    {
        map textures/effects/envmap_slate_90.tga
        rgbGen lightingdiffuse
        tcGen environment
    }
    {
        map models/multiplayer/landmine_bbetty/landmine.jpg
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_bbetty/marker_allies
{
    cull disable
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine_bbetty/marker_allies.jpg
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_bbetty/marker_axis
{
    cull disable
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine_bbetty/marker_axis.jpg
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_pgas/landmine
{
    {
        map textures/effects/envmap_slate_90.tga
        rgbGen lightingdiffuse
        tcGen environment
    }
    {
        map models/multiplayer/landmine_pgas/landmine.jpg
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_pgas/marker_allies
{
    cull disable
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine_pgas/marker_allies.jpg
        rgbGen lightingdiffuse
    }
}

models/multiplayer/landmine_pgas/marker_axis
{
    cull disable
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine_pgas/marker_axis.jpg
        rgbGen lightingdiffuse
    }
}

gfx/limbo/skill_4pieces_on_5
{
	nomipmaps
	nopicmip
	{
		map gfx/limbo/skill_4pieces_5.tga
		blendfunc blend
		rgbgen vertex
	}
}

gfx/limbo/skill_4pieces_off_5
{
	nomipmaps
	nopicmip
	{
		map gfx/limbo/skill_4pieces_off_5.tga
		blendfunc blend
		rgbgen vertex
	}
}

sprites/muted
{
	nocompress
	nopicmip
	{
		map sprites/muted.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

gfx/limbo/cpm_dynamite
{
	nocompress
	nopicmip
	nomipmaps
	{
		map gfx/limbo/cm_dynamite.tga
		blendfunc blend
		rgbGen vertex
		alphaGen vertex
	}
}

models/gibs/gibs
{
	{
		map models/gibs/gibs.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingDiffuse 
	}
	{
		map models/gibs/gibs.jpg
		blendFunc GL_ZERO GL_ONE
		rgbGen lightingDiffuse 
	}
}

models/multiplayer/landmine_bbetty/icon
{
    nomipmaps
    nopicmip
    {
	    map models/multiplayer/landmine_bbetty/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

models/multiplayer/landmine_pgas/icon
{
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine_pgas/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

models/multiplayer/poison_gas/icon
{
    nomipmaps
    nopicmip
    {
        map models/multiplayer/poison_gas/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

models/multiplayer/landmine/icon
{
    nomipmaps
    nopicmip
    {
        map models/multiplayer/landmine/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_syringe2_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_syringe2_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_syringe_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_syringe_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_binoculars_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_binoculars_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_mortar_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_mortar_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_mg42_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_mg42_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_satchel_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_satchel_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_fg42_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_fg42_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

icons/iconw_radio_1_select
{
    nomipmaps
    nopicmip
    {
        map icons/iconw_radio_1_select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}
