/**
 * Copyright (c) 2006-2019 LOVE Development Team
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

#include "wrap_ImageData.h"

#include "data/wrap_Data.h"
#include "filesystem/File.h"
#include "filesystem/Filesystem.h"

// Shove the wrap_ImageData.lua code directly into a raw string literal.
static const char imagedata_lua[] =
#include "wrap_ImageData.lua"
;

namespace love
{
namespace image
{

/**
 * NOTE: Additional wrapper code is in wrap_ImageData.lua. Be sure to keep it
 * in sync with any changes made to this file!
 **/

ImageData *luax_checkimagedata(lua_State *L, int idx)
{
	return luax_checktype<ImageData>(L, idx);
}

int w_ImageData_clone(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1), *c = nullptr;
	luax_catchexcept(L, [&](){ c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

int w_ImageData_getFormat(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	PixelFormat format = t->getFormat();
	const char *fstr = nullptr;

	if (!getConstant(format, fstr))
		return luaL_error(L, "Unknown pixel format.");

	lua_pushstring(L, fstr);
	return 1;
}

int w_ImageData_getWidth(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getWidth());
	return 1;
}

int w_ImageData_getHeight(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getHeight());
	return 1;
}

int w_ImageData_getDimensions(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getWidth());
	lua_pushinteger(L, t->getHeight());
	return 2;
}

template <int components>
static void luax_checkpixel_unorm8(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < std::min(components, 3); i++)
		p.rgba8[i] = (uint8) ((luax_checknumberclamped01(L, startidx + i) * 255.0) + 0.5);
	if (components > 3)
		p.rgba8[3] = (uint8) ((luax_optnumberclamped01(L, startidx + 3, 1.0) * 255.0) + 0.5);
}

template <int components>
static void luax_checkpixel_unorm16(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < std::min(components, 3); i++)
		p.rgba16[i] = (uint16) ((luax_checknumberclamped01(L, startidx + i) * 65535.0) + 0.5);
	if (components > 3)
		p.rgba16[3] = (uint16) ((luax_optnumberclamped01(L, startidx + 3, 1.0) * 65535.0) + 0.5);
}

template <int components>
static void luax_checkpixel_float16(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < std::min(components, 3); i++)
		p.rgba16f[i] = float32to16((float) luaL_checknumber(L, startidx + i));
	 if (components > 3)
		p.rgba16f[3] = float32to16((float) luaL_optnumber(L, startidx + 3, 1.0));
}

template <int components>
static void luax_checkpixel_float32(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < std::min(components, 3); i++)
		p.rgba32f[i] = (float) luaL_checknumber(L, startidx + i);
	if (components > 3)
		p.rgba32f[3] = (float) luaL_optnumber(L, startidx + 3, 1.0);
}

static void luax_checkpixel_r8(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm8<1>(L, startidx, p);
}

static void luax_checkpixel_rg8(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm8<2>(L, startidx, p);
}

static void luax_checkpixel_rgba8(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm8<4>(L, startidx, p);
}

static void luax_checkpixel_r16(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm16<1>(L, startidx, p);
}

static void luax_checkpixel_rg16(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm16<2>(L, startidx, p);
}

static void luax_checkpixel_rgba16(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_unorm16<4>(L, startidx, p);
}

static void luax_checkpixel_r16f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float16<1>(L, startidx, p);
}

static void luax_checkpixel_rg16f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float16<2>(L, startidx, p);
}

static void luax_checkpixel_rgba16f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float16<4>(L, startidx, p);
}

static void luax_checkpixel_r32f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float32<1>(L, startidx, p);
}

static void luax_checkpixel_rg32f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float32<2>(L, startidx, p);
}

static void luax_checkpixel_rgba32f(lua_State *L, int startidx, Pixel &p)
{
	luax_checkpixel_float32<4>(L, startidx, p);
}

static void luax_checkpixel_rgba4(lua_State *L, int startidx, Pixel &p)
{
	// LSB->MSB: [a, b, g, r]
	uint16 r = (uint16) ((luax_checknumberclamped01(L, startidx + 0) * 0xF) + 0.5);
	uint16 g = (uint16) ((luax_checknumberclamped01(L, startidx + 1) * 0xF) + 0.5);
	uint16 b = (uint16) ((luax_checknumberclamped01(L, startidx + 2) * 0xF) + 0.5);
	uint16 a = (uint16) ((luax_optnumberclamped01(L, startidx + 3, 1.0) * 0xF) + 0.5);
	p.packed16 = (r << 12) | (g << 8) | (b << 4) | (a << 0);
}

static void luax_checkpixel_rgb5a1(lua_State *L, int startidx, Pixel &p)
{
	// LSB->MSB: [a, b, g, r]
	uint16 r = (uint16) ((luax_checknumberclamped01(L, startidx + 0) * 0x1F) + 0.5);
	uint16 g = (uint16) ((luax_checknumberclamped01(L, startidx + 1) * 0x1F) + 0.5);
	uint16 b = (uint16) ((luax_checknumberclamped01(L, startidx + 2) * 0x1F) + 0.5);
	uint16 a = (uint16) ((luax_optnumberclamped01(L, startidx + 3, 1.0) * 0x1) + 0.5);
	p.packed16 = (r << 11) | (g << 6) | (b << 1) | (a << 0);
}

static void luax_checkpixel_rgb565(lua_State *L, int startidx, Pixel &p)
{
	// LSB->MSB: [b, g, r]
	uint16 r = (uint16) ((luax_checknumberclamped01(L, startidx + 0) * 0x1F) + 0.5);
	uint16 g = (uint16) ((luax_checknumberclamped01(L, startidx + 1) * 0x3F) + 0.5);
	uint16 b = (uint16) ((luax_checknumberclamped01(L, startidx + 2) * 0x1F) + 0.5);
	p.packed16 = (r << 11) | (g << 5) | (b << 0);
}

static void luax_checkpixel_rgb10a2(lua_State *L, int startidx, Pixel &p)
{
	// LSB->MSB: [r, g, b, a]
	uint32 r = (uint32) ((luax_checknumberclamped01(L, startidx + 0) * 0x3FF) + 0.5);
	uint32 g = (uint32) ((luax_checknumberclamped01(L, startidx + 1) * 0x3FF) + 0.5);
	uint32 b = (uint32) ((luax_checknumberclamped01(L, startidx + 2) * 0x3FF) + 0.5);
	uint32 a = (uint32) ((luax_optnumberclamped01(L, startidx + 3, 1.0) * 0x3) + 0.5);
	p.packed32 = (r << 0) | (g << 10) | (b << 20) | (a << 30);
}

static void luax_checkpixel_rg11b10f(lua_State *L, int startidx, Pixel &p)
{
	// LSB->MSB: [r, g, b]
	float11 r = float32to11((float) luaL_checknumber(L, startidx + 0));
	float11 g = float32to11((float) luaL_checknumber(L, startidx + 1));
	float10 b = float32to10((float) luaL_checknumber(L, startidx + 2));
	p.packed32 = (r << 0) | (g << 11) | (b << 22);
}

static lua_Number fillValues[] = {0.0, 0.0, 0.0, 1.0};

template <int components>
static int luax_pushpixel_unorm8(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) p.rgba8[i] / 255.0);
	for (int i = components; i < 4; i++)
		lua_pushnumber(L, fillValues[i]);
	return 4;
}

template <int components>
static int luax_pushpixel_unorm16(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) p.rgba16[i] / 65535.0);
	for (int i = components; i < 4; i++)
		lua_pushnumber(L, fillValues[i]);
	return 4;
}

template <int components>
static int luax_pushpixel_float16(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) float16to32(p.rgba16f[i]));
	for (int i = components; i < 4; i++)
		lua_pushnumber(L, fillValues[i]);
	return 4;
}

template <int components>
static int luax_pushpixel_float32(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) p.rgba32f[i]);
	for (int i = components; i < 4; i++)
		lua_pushnumber(L, fillValues[i]);
	return 4;
}

static int luax_pushpixel_r8(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm8<1>(L, p);
}

static int luax_pushpixel_rg8(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm8<2>(L, p);
}

static int luax_pushpixel_rgba8(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm8<4>(L, p);
}

static int luax_pushpixel_r16(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm16<1>(L, p);
}

static int luax_pushpixel_rg16(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm16<2>(L, p);
}

static int luax_pushpixel_rgba16(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_unorm16<4>(L, p);
}

static int luax_pushpixel_r16f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float16<1>(L, p);
}

static int luax_pushpixel_rg16f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float16<2>(L, p);
}

static int luax_pushpixel_rgba16f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float16<4>(L, p);
}

static int luax_pushpixel_r32f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float32<1>(L, p);
}

static int luax_pushpixel_rg32f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float32<2>(L, p);
}

static int luax_pushpixel_rgba32f(lua_State *L, const Pixel &p)
{
	return luax_pushpixel_float32<4>(L, p);
}

static int luax_pushpixel_rgba4(lua_State *L, const Pixel &p)
{
	// LSB->MSB: [a, b, g, r]
	lua_pushnumber(L, ((p.packed16 >> 12) & 0xF) / (double)0xF);
	lua_pushnumber(L, ((p.packed16 >>  8) & 0xF) / (double)0xF);
	lua_pushnumber(L, ((p.packed16 >>  4) & 0xF) / (double)0xF);
	lua_pushnumber(L, ((p.packed16 >>  0) & 0xF) / (double)0xF);
	return 4;
}

static int luax_pushpixel_rgb5a1(lua_State *L, const Pixel &p)
{
	// LSB->MSB: [a, b, g, r]
	lua_pushnumber(L, ((p.packed16 >> 11) & 0x1F) / (double)0x1F);
	lua_pushnumber(L, ((p.packed16 >>  6) & 0x1F) / (double)0x1F);
	lua_pushnumber(L, ((p.packed16 >>  1) & 0x1F) / (double)0x1F);
	lua_pushnumber(L, ((p.packed16 >>  0) & 0x1)  / (double)0x1);
	return 4;
}

static int luax_pushpixel_rgb565(lua_State *L, const Pixel &p)
{
	// LSB->MSB: [b, g, r]
	lua_pushnumber(L, ((p.packed16 >> 11) & 0x1F) / (double)0x1F);
	lua_pushnumber(L, ((p.packed16 >>  5) & 0x3F) / (double)0x3F);
	lua_pushnumber(L, ((p.packed16 >>  0) & 0x1F) / (double)0x1F);
	lua_pushnumber(L, 1.0);
	return 4;
}

static int luax_pushpixel_rgb10a2(lua_State *L, const Pixel &p)
{
	// LSB->MSB: [r, g, b, a]
	lua_pushnumber(L, ((p.packed32 >>  0) & 0x3FF) / (double)0x3FF);
	lua_pushnumber(L, ((p.packed32 >> 10) & 0x3FF) / (double)0x3FF);
	lua_pushnumber(L, ((p.packed32 >> 20) & 0x3FF) / (double)0x3FF);
	lua_pushnumber(L, ((p.packed32 >> 30) & 0x3)   / (double)0x3);
	return 4;
}

static int luax_pushpixel_rg11b10f(lua_State *L, const Pixel &p)
{
	// LSB->MSB: [r, g, b]
	lua_pushnumber(L, float11to32((float11) ((p.packed32 >>  0) & 0x7FF)));
	lua_pushnumber(L, float11to32((float11) ((p.packed32 >> 11) & 0x7FF)));
	lua_pushnumber(L, float10to32((float10) ((p.packed32 >> 22) & 0x3FF)));
	lua_pushnumber(L, 1.0);
	return 4;
}

typedef void(*checkpixel)(lua_State *L, int startidx, Pixel &p);
typedef int(*pushpixel)(lua_State *L, const Pixel &p);

static checkpixel checkFormats[PIXELFORMAT_MAX_ENUM] = {};
static pushpixel pushFormats[PIXELFORMAT_MAX_ENUM] = {};

int w_ImageData_getPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checkinteger(L, 2);
	int y = (int) luaL_checkinteger(L, 3);

	PixelFormat format = t->getFormat();

	Pixel p;
	luax_catchexcept(L, [&](){ t->getPixel(x, y, p); });

	return pushFormats[format](L, p);
}

int w_ImageData_setPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checkinteger(L, 2);
	int y = (int) luaL_checkinteger(L, 3);

	PixelFormat format = t->getFormat();

	Pixel p;

	if (lua_istable(L, 4))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 4, i);

		checkFormats[format](L, -4, p);

		lua_pop(L, 4);
	}
	else
		checkFormats[format](L, 4, p);

	luax_catchexcept(L, [&](){ t->setPixel(x, y, p); });
	return 0;
}

// ImageData:mapPixel. Not thread-safe! See wrap_ImageData.lua for the thread-
// safe wrapper function.
int w_ImageData__mapPixelUnsafe(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	// No optints because we assume they're done in the wrapper function.
	int sx = (int) lua_tonumber(L, 3);
	int sy = (int) lua_tonumber(L, 4);
	int w  = (int) lua_tonumber(L, 5);
	int h  = (int) lua_tonumber(L, 6);

	if (!(t->inside(sx, sy) && t->inside(sx+w-1, sy+h-1)))
		return luaL_error(L, "Invalid rectangle dimensions.");

	int iw = t->getWidth();

	PixelFormat format = t->getFormat();

	auto checkpixel = checkFormats[format];
	auto pushpixel = pushFormats[format];

	uint8 *data = (uint8 *) t->getData();
	size_t pixelsize = t->getPixelSize();

	for (int y = sy; y < sy+h; y++)
	{
		for (int x = sx; x < sx+w; x++)
		{
			Pixel *pixeldata = (Pixel *) (data + (y * iw + x) * pixelsize);

			lua_pushvalue(L, 2); // ImageData
			lua_pushnumber(L, x);
			lua_pushnumber(L, y);

			pushpixel(L, *pixeldata);

			lua_call(L, 6, 4);

			checkpixel(L, -4, *pixeldata);
			lua_pop(L, 4); // Pop return values.
		}
	}

	return 0;
}

int w_ImageData_paste(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	ImageData *src = luax_checkimagedata(L, 2);
	int dx = (int) luaL_checkinteger(L, 3);
	int dy = (int) luaL_checkinteger(L, 4);
	int sx = (int) luaL_optinteger(L, 5, 0);
	int sy = (int) luaL_optinteger(L, 6, 0);
	int sw = (int) luaL_optinteger(L, 7, src->getWidth());
	int sh = (int) luaL_optinteger(L, 8, src->getHeight());
	t->paste((love::image::ImageData *)src, dx, dy, sx, sy, sw, sh);
	return 0;
}

int w_ImageData_encode(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);

	FormatHandler::EncodedFormat format;
	const char *fmt = luaL_checkstring(L, 2);
	if (!ImageData::getConstant(fmt, format))
		return luax_enumerror(L, "encoded image format", ImageData::getConstants(format), fmt);

	bool hasfilename = false;

	std::string filename = "Image." + std::string(fmt);
	if (!lua_isnoneornil(L, 3))
	{
		hasfilename = true;
		filename = luax_checkstring(L, 3);
	}

	love::filesystem::FileData *filedata = nullptr;
	luax_catchexcept(L, [&](){ filedata = t->encode(format, filename.c_str(), hasfilename); });

	luax_pushtype(L, filedata);
	filedata->release();

	return 1;
}

int w_ImageData__performAtomic(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int err = 0;

	{
		love::thread::Lock lock(t->getMutex());
		// call the function, passing any user-specified arguments.
		err = lua_pcall(L, lua_gettop(L) - 2, LUA_MULTRET, 0);
	}

	// Unfortunately, this eats the stack trace, too bad.
	if (err != 0)
		return lua_error(L);

	// The function and everything after it in the stack are eaten by the pcall,
	// leaving only the ImageData object. Everything else is a return value.
	return lua_gettop(L) - 1;
}

// C functions in a struct, necessary for the FFI versions of ImageData methods.
struct FFI_ImageData
{
	void (*lockMutex)(Proxy *p);
	void (*unlockMutex)(Proxy *p);

	float (*float16to32)(float16 f);
	float16 (*float32to16)(float f);

	float (*float11to32)(float11 f);
	float11 (*float32to11)(float f);

	float (*float10to32)(float10 f);
	float10 (*float32to10)(float f);
};

static FFI_ImageData ffifuncs =
{
	[](Proxy *p) // lockMutex
	{
		// We don't do any type-checking for the Proxy here since these functions
		// are always called from code which has already done type checking.
		ImageData *i = (ImageData *) p->object;
		i->getMutex()->lock();
	},

	[](Proxy *p) // unlockMutex
	{
		ImageData *i = (ImageData *) p->object;
		i->getMutex()->unlock();
	},

	float16to32,
	float32to16,
	float11to32,
	float32to11,
	float10to32,
	float32to10,
};

static const luaL_Reg w_ImageData_functions[] =
{
	{ "clone", w_ImageData_clone },
	{ "getFormat", w_ImageData_getFormat },
	{ "getWidth", w_ImageData_getWidth },
	{ "getHeight", w_ImageData_getHeight },
	{ "getDimensions", w_ImageData_getDimensions },
	{ "getPixel", w_ImageData_getPixel },
	{ "setPixel", w_ImageData_setPixel },
	{ "paste", w_ImageData_paste },
	{ "encode", w_ImageData_encode },

	// Used in the Lua wrapper code.
	{ "_mapPixelUnsafe", w_ImageData__mapPixelUnsafe },
	{ "_performAtomic", w_ImageData__performAtomic },

	{ 0, 0 }
};

extern "C" int luaopen_imagedata(lua_State *L)
{
	checkFormats[PIXELFORMAT_R8]       = luax_checkpixel_r8;
	checkFormats[PIXELFORMAT_RG8]      = luax_checkpixel_rg8;
	checkFormats[PIXELFORMAT_RGBA8]    = luax_checkpixel_rgba8;
	checkFormats[PIXELFORMAT_R16]      = luax_checkpixel_r16;
	checkFormats[PIXELFORMAT_RG16]     = luax_checkpixel_rg16;
	checkFormats[PIXELFORMAT_RGBA16]   = luax_checkpixel_rgba16;
	checkFormats[PIXELFORMAT_R16F]     = luax_checkpixel_r16f;
	checkFormats[PIXELFORMAT_RG16F]    = luax_checkpixel_rg16f;
	checkFormats[PIXELFORMAT_RGBA16F]  = luax_checkpixel_rgba16f;
	checkFormats[PIXELFORMAT_R32F]     = luax_checkpixel_r32f;
	checkFormats[PIXELFORMAT_RG32F]    = luax_checkpixel_rg32f;
	checkFormats[PIXELFORMAT_RGBA32F]  = luax_checkpixel_rgba32f;
	checkFormats[PIXELFORMAT_RGBA4]    = luax_checkpixel_rgba4;
	checkFormats[PIXELFORMAT_RGB5A1]   = luax_checkpixel_rgb5a1;
	checkFormats[PIXELFORMAT_RGB565]   = luax_checkpixel_rgb565;
	checkFormats[PIXELFORMAT_RGB10A2]  = luax_checkpixel_rgb10a2;
	checkFormats[PIXELFORMAT_RG11B10F] = luax_checkpixel_rg11b10f;

	pushFormats[PIXELFORMAT_R8]       = luax_pushpixel_r8;
	pushFormats[PIXELFORMAT_RG8]      = luax_pushpixel_rg8;
	pushFormats[PIXELFORMAT_RGBA8]    = luax_pushpixel_rgba8;
	pushFormats[PIXELFORMAT_R16]      = luax_pushpixel_r16;
	pushFormats[PIXELFORMAT_RG16]     = luax_pushpixel_rg16;
	pushFormats[PIXELFORMAT_RGBA16]   = luax_pushpixel_rgba16;
	pushFormats[PIXELFORMAT_R16F]     = luax_pushpixel_r16f;
	pushFormats[PIXELFORMAT_RG16F]    = luax_pushpixel_rg16f;
	pushFormats[PIXELFORMAT_RGBA16F]  = luax_pushpixel_rgba16f;
	pushFormats[PIXELFORMAT_R32F]     = luax_pushpixel_r32f;
	pushFormats[PIXELFORMAT_RG32F]    = luax_pushpixel_rg32f;
	pushFormats[PIXELFORMAT_RGBA32F]  = luax_pushpixel_rgba32f;
	pushFormats[PIXELFORMAT_RGBA4]    = luax_pushpixel_rgba4;
	pushFormats[PIXELFORMAT_RGB5A1]   = luax_pushpixel_rgb5a1;
	pushFormats[PIXELFORMAT_RGB565]   = luax_pushpixel_rgb565;
	pushFormats[PIXELFORMAT_RGB10A2]  = luax_pushpixel_rgb10a2;
	pushFormats[PIXELFORMAT_RG11B10F] = luax_pushpixel_rg11b10f;

	int ret = luax_register_type(L, &ImageData::type, data::w_Data_functions, w_ImageData_functions, nullptr);

	love::data::luax_rundatawrapper(L, ImageData::type);
	luax_runwrapper(L, imagedata_lua, sizeof(imagedata_lua), "ImageData.lua", ImageData::type, &ffifuncs);

	return ret;
}

} // image
} // love
