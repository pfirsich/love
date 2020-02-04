/**
 * Copyright (c) 2006-2020 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "wrap_Texture.h"

namespace love
{
namespace graphics
{

Texture *luax_checktexture(lua_State *L, int idx)
{
	return luax_checktype<Texture>(L, idx);
}

int w_Texture_getTextureType(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const char *tstr;
	if (!Texture::getConstant(t->getTextureType(), tstr))
		return luax_enumerror(L, "texture type", Texture::getConstants(TEXTURE_MAX_ENUM), tstr);
	lua_pushstring(L, tstr);
	return 1;
}

static int w__optMipmap(lua_State *L, Texture *t, int idx)
{
	int mipmap = 0;

	if (!lua_isnoneornil(L, idx))
	{
		mipmap = (int) luaL_checkinteger(L, idx) - 1;

		if (mipmap < 0 || mipmap >= t->getMipmapCount())
			luaL_error(L, "Invalid mipmap index: %d", mipmap + 1);
	}

	return mipmap;
}

int w_Texture_getWidth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getWidth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getHeight(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getHeight(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getDimensions(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	int mipmap = w__optMipmap(L, t, 2);
	lua_pushnumber(L, t->getWidth(mipmap));
	lua_pushnumber(L, t->getHeight(mipmap));
	return 2;
}

int w_Texture_getDepth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getDepth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getLayerCount(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getLayerCount());
	return 1;
}

int w_Texture_getMipmapCount(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getMipmapCount());
	return 1;
}

int w_Texture_getPixelWidth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getPixelWidth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getPixelHeight(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getPixelHeight(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getPixelDimensions(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	int mipmap = w__optMipmap(L, t, 2);
	lua_pushnumber(L, t->getPixelWidth(mipmap));
	lua_pushnumber(L, t->getPixelHeight(mipmap));
	return 2;
}

int w_Texture_getDPIScale(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getDPIScale());
	return 1;
}

int w_Texture_isFormatLinear(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	luax_pushboolean(L, t->isFormatLinear());
	return 1;
}

int w_Texture_isCompressed(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	luax_pushboolean(L, t->isCompressed());
	return 1;
}

int w_Texture_getMSAA(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushinteger(L, t->getMSAA());
	return 1;
}

int w_Texture_setFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	SamplerState s = t->getSamplerState();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!SamplerState::getConstant(minstr, s.minFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.minFilter), minstr);
	if (!SamplerState::getConstant(magstr, s.magFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.magFilter), magstr);

	s.maxAnisotropy = std::min(std::max(1, (int) luaL_optnumber(L, 4, 1.0)), LOVE_UINT8_MAX);

	luax_catchexcept(L, [&](){ t->setSamplerState(s); });
	return 0;
}

int w_Texture_getFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const SamplerState &s = t->getSamplerState();

	const char *minstr = nullptr;
	const char *magstr = nullptr;

	if (!SamplerState::getConstant(s.minFilter, minstr))
		return luaL_error(L, "Unknown filter mode.");
	if (!SamplerState::getConstant(s.magFilter, magstr))
		return luaL_error(L, "Unknown filter mode.");

	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, s.maxAnisotropy);
	return 3;
}

int w_Texture_setMipmapFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	SamplerState s = t->getSamplerState();

	// Mipmapping is disabled if no argument is given.
	if (lua_isnoneornil(L, 2))
		s.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;
	else
	{
		const char *mipmapstr = luaL_checkstring(L, 2);
		if (!SamplerState::getConstant(mipmapstr, s.mipmapFilter))
			return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.mipmapFilter), mipmapstr);
	}

	s.lodBias = -((float) luaL_optnumber(L, 3, 0.0));

	luax_catchexcept(L, [&](){ t->setSamplerState(s); });
	return 0;
}

int w_Texture_getMipmapFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const SamplerState &s = t->getSamplerState();

	const char *mipmapstr;
	if (SamplerState::getConstant(s.mipmapFilter, mipmapstr))
		lua_pushstring(L, mipmapstr);
	else
		lua_pushnil(L); // only return a mipmap filter if mipmapping is enabled

	lua_pushnumber(L, -s.lodBias);
	return 2;
}

int w_Texture_setWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	SamplerState s = t->getSamplerState();

	const char *sstr = luaL_checkstring(L, 2);
	const char *tstr = luaL_optstring(L, 3, sstr);
	const char *rstr = luaL_optstring(L, 4, sstr);

	if (!SamplerState::getConstant(sstr, s.wrapU))
		return luax_enumerror(L, "wrap mode", SamplerState::getConstants(s.wrapU), sstr);
	if (!SamplerState::getConstant(tstr, s.wrapV))
		return luax_enumerror(L, "wrap mode", SamplerState::getConstants(s.wrapV), tstr);
	if (!SamplerState::getConstant(rstr, s.wrapW))
		return luax_enumerror(L, "wrap mode", SamplerState::getConstants(s.wrapW), rstr);

	luax_catchexcept(L, [&](){ t->setSamplerState(s); });
	return 1;
}

int w_Texture_getWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const SamplerState &s = t->getSamplerState();

	const char *sstr = nullptr;
	const char *tstr = nullptr;
	const char *rstr = nullptr;

	if (!SamplerState::getConstant(s.wrapU, sstr))
		return luaL_error(L, "Unknown wrap mode.");
	if (!SamplerState::getConstant(s.wrapV, tstr))
		return luaL_error(L, "Unknown wrap mode.");
	if (!SamplerState::getConstant(s.wrapW, rstr))
		return luaL_error(L, "Unknown wrap mode.");

	lua_pushstring(L, sstr);
	lua_pushstring(L, tstr);
	lua_pushstring(L, rstr);
	return 3;
}

int w_Texture_getFormat(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	PixelFormat format = t->getPixelFormat();
	const char *str;
	if (!getConstant(format, str))
		return luaL_error(L, "Unknown pixel format.");

	lua_pushstring(L, str);
	return 1;
}

int w_Texture_isReadable(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	luax_pushboolean(L, t->isReadable());
	return 1;
}

int w_Texture_setDepthSampleMode(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	SamplerState s = t->getSamplerState();

	s.depthSampleMode.hasValue = false;
	if (!lua_isnoneornil(L, 2))
	{
		const char *str = luaL_checkstring(L, 2);

		s.depthSampleMode.hasValue = true;
		if (!getConstant(str, s.depthSampleMode.value))
			return luax_enumerror(L, "compare mode", getConstants(s.depthSampleMode.value), str);
	}

	luax_catchexcept(L, [&](){ t->setSamplerState(s); });
	return 0;
}

int w_Texture_getDepthSampleMode(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const SamplerState &s = t->getSamplerState();

	if (s.depthSampleMode.hasValue)
	{
		const char *str = nullptr;
		if (!getConstant(s.depthSampleMode.value, str))
			return luaL_error(L, "Unknown compare mode.");
		lua_pushstring(L, str);
	}
	else
		lua_pushnil(L);

	return 1;
}

int w_Texture_generateMipmaps(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	luax_catchexcept(L, [&]() { t->generateMipmaps(); });
	return 0;
}

int w_Texture_replacePixels(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	love::image::ImageData *id = luax_checktype<love::image::ImageData>(L, 2);

	int slice = 0;
	int mipmap = 0;
	int x = 0;
	int y = 0;
	bool reloadmipmaps = false; // TODO i->getMipmapsSource() == Texture::MIPMAPS_SOURCE_GENERATED;

	if (t->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 3) - 1;

	mipmap = (int) luaL_optinteger(L, 4, 1) - 1;

	if (!lua_isnoneornil(L, 5))
	{
		x = (int) luaL_checkinteger(L, 5);
		y = (int) luaL_checkinteger(L, 6);

		if (reloadmipmaps)
			reloadmipmaps = luax_optboolean(L, 7, reloadmipmaps);
	}

	luax_catchexcept(L, [&](){ t->replacePixels(id, slice, mipmap, x, y, reloadmipmaps); });
	return 0;
}

int w_Texture_newImageData(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, love::image::Image::type);

	int slice = 0;
	int mipmap = 0;
	Rect rect = {0, 0, t->getPixelWidth(), t->getPixelHeight()};

	if (t->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 2) - 1;

	mipmap = (int) luaL_optinteger(L, 3, 1) - 1;

	if (!lua_isnoneornil(L, 4))
	{
		rect.x = (int) luaL_checkinteger(L, 4);
		rect.y = (int) luaL_checkinteger(L, 5);
		rect.w = (int) luaL_checkinteger(L, 6);
		rect.h = (int) luaL_checkinteger(L, 7);
	}

	love::image::ImageData *img = nullptr;
	luax_catchexcept(L, [&](){ img = t->newImageData(image, slice, mipmap, rect); });

	luax_pushtype(L, img);
	img->release();
	return 1;
}

const luaL_Reg w_Texture_functions[] =
{
	{ "getTextureType", w_Texture_getTextureType },
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "getDepth", w_Texture_getDepth },
	{ "getLayerCount", w_Texture_getLayerCount },
	{ "getMipmapCount", w_Texture_getMipmapCount },
	{ "getPixelWidth", w_Texture_getPixelWidth },
	{ "getPixelHeight", w_Texture_getPixelHeight },
	{ "getPixelDimensions", w_Texture_getPixelDimensions },
	{ "getDPIScale", w_Texture_getDPIScale },
	{ "isFormatLinear", w_Texture_isFormatLinear },
	{ "isCompressed", w_Texture_isCompressed },
	{ "getMSAA", w_Texture_getMSAA },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setMipmapFilter", w_Texture_setMipmapFilter },
	{ "getMipmapFilter", w_Texture_getMipmapFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },
	{ "getFormat", w_Texture_getFormat },
	{ "isReadable", w_Texture_isReadable },
	{ "getDepthSampleMode", w_Texture_getDepthSampleMode },
	{ "setDepthSampleMode", w_Texture_setDepthSampleMode },
	{ "generateMipmaps", w_Texture_generateMipmaps },
	{ "replacePixels", w_Texture_replacePixels },
	{ "newImageData", w_Texture_newImageData },
	{ 0, 0 }
};

extern "C" int luaopen_texture(lua_State *L)
{
	return luax_register_type(L, &Texture::type, w_Texture_functions, nullptr);
}

} // graphics
} // love
