// Winchester Shaders

models/weapons2/m97/shotgunBarrel
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/m97/shotgunBarrel.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/m97/shotgunStock
{
	{
		map textures/effects/envmap_slate.tga
		rgbGen lightingdiffuse
		tcGen environment
	}
	{
		map models/weapons2/m97/shotgunStock.tga
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}
}

models/weapons2/m97/icon
{
    nomipmaps
    nopicmip
    {
        map models/weapons2/m97/icon.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}
