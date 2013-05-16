//								-*- c++ -*-

#include "bitmap.h"
#include "setup.h"
#include <vdr/tools.h>
#define X_DISPLAY_MISSING
#ifdef HAVE_IMAGEMAGICK
#include <Magick++.h>
using namespace Magick;
#elif defined(HAVE_IMLIB2)
#include "quantize.h"
#include <Imlib2.h>
#endif
#include <glob.h>

cBitmapCache cText2SkinBitmap::mCache(Text2SkinSetup.MaxCacheFill);

void cBitmapCache::DeleteObject(const tBitmapSpec &Key, cText2SkinBitmap *&Data)
{
	delete Data;
}

void cBitmapCache::ResetObject(cText2SkinBitmap *&Data)
{
	Data->Reset();
}


cText2SkinBitmap *cText2SkinBitmap::Load(const std::string &Filename, int Alpha, int height,
                                         int width, int colors, bool Quiet) {
	tBitmapSpec spec(Filename, Alpha, height, width, colors);
	Dprintf("checking image with spec %s_%d_%d_%d_%d..", Filename.c_str(),Alpha,height,width,colors);
	std::string fname = Filename;

	cText2SkinBitmap *res = NULL;
	if (mCache.Contains(spec)) {
		res = mCache[spec];
		Dprintf("..cache ok\n");
	} else {
		if (fname.find('*') != std::string::npos) {
			glob_t gbuf;
			if (glob(fname.c_str(), 0, NULL, &gbuf) == 0){
				Dprintf("GLOB: FOUND %s\n", gbuf.gl_pathv[0]);
				fname = gbuf.gl_pathv[0];
			} else {
				if (!Quiet)
					esyslog("ERROR: text2skin: No match for wildcard filename %s",
							Filename.c_str());
				fname = "";
			}
			globfree(&gbuf);
		}

		res = new cText2SkinBitmap;
		int len = fname.length();
		bool result = false;
		if (len > 4) {
			if (fname.substr(len - 4, 4) == ".xpm")
				result = res->LoadXpm(fname.c_str());
			else {
				result = res->LoadNonXpm(fname.c_str(), height, width, colors, Quiet);
			}
		} else if (!Quiet)
			esyslog("ERROR: text2skin: filename %s too short to identify format", fname.c_str());

		Dprintf("..load %sok\n", result ? "" : "not ");
		if (result)
			res->SetAlpha(Alpha);
		else
			DELETENULL(res);
		mCache[spec] = res;
	}
	return res;
}

bool cText2SkinBitmap::Available(const std::string &Filename, int Alpha, int height, int width,
                                 int colors)
{
	if ((int)Filename.find('*') != -1) {
		bool result = false;
		glob_t gbuf;
		if (glob(Filename.c_str(), 0, NULL, &gbuf) == 0)
			result = true;
		globfree(&gbuf);
		return result;
	} else
		return access(Filename.c_str(), F_OK) == 0;
}

void cText2SkinBitmap::Init(void) {
#ifdef HAVE_IMAGEMAGICK
	InitializeMagick(NULL);
#endif
	mCache.SetMaxItems(Text2SkinSetup.MaxCacheFill);
}

cText2SkinBitmap::cText2SkinBitmap(void):
	mCurrent(0),
	mLastGet(0)
{
}

cText2SkinBitmap::~cText2SkinBitmap() {
	for (int i = 0; i < (int)mBitmaps.size(); ++i)
		delete mBitmaps[i];
	mBitmaps.clear();
}

cBitmap &cText2SkinBitmap::Get(uint &UpdateIn, uint Now) {
	if (mBitmaps.size() == 1)
		return *mBitmaps[0];

	time_t upd;
	int diff;
	if (mLastGet == 0) {
		mLastGet = Now;
		upd = mDelay;
	} else if ((diff = Now - mLastGet) >= mDelay) {
		mCurrent = (mCurrent + 1) % mBitmaps.size();
		mLastGet = Now;
		upd = mDelay - diff > 1 ? mDelay - diff : 1;
	} else {
		upd = mDelay - diff;
	}

	if (UpdateIn == 0 || UpdateIn > (uint)upd)
		UpdateIn = upd;

	return *mBitmaps[mCurrent];
}

void cText2SkinBitmap::SetAlpha(int Alpha) {
	if (Alpha > 0) {
		std::vector<cBitmap*>::iterator it = mBitmaps.begin();
		for (; it != mBitmaps.end(); ++it) {
			int count;
			if ((*it)->Colors(count)) {
				for (int i = 0; i < count; ++i) {
					int alpha = (((*it)->Color(i) & 0xFF000000) >> 24) * Alpha / 255;
					(*it)->SetColor(i, ((*it)->Color(i) & 0x00FFFFFF) | (alpha << 24));
				}
			}
		}
	}
}

bool cText2SkinBitmap::LoadXpm(const char *Filename) {
	cBitmap *bmp = new cBitmap(1,1,1);
	if (bmp->LoadXpm(Filename)) {
		mBitmaps.push_back(bmp);
		return true;
	}
	delete bmp;
	return false;
}

bool cText2SkinBitmap::LoadNonXpm(const char *Filename, int height, int width, int colors, bool Quiet) {

#ifdef HAVE_IMAGEMAGICK
	std::vector<Image> images;
	cBitmap *bmp = NULL;
	try {
		int w, h;
		std::vector<Image>::iterator it;
		readImages(&images, Filename);
		if (images.empty()) {
			esyslog("ERROR: text2skin: Couldn't load %s", Filename);
			return false;
		}
		mDelay = images[0].animationDelay() * 10;
		for (it = images.begin(); it != images.end(); ++it) {
			if (colors != 0){
				(*it).opacity(OpaqueOpacity);
				(*it).backgroundColor( Color ( 0,0,0,0) );
				(*it).quantizeColorSpace( RGBColorspace );
				(*it).quantizeColors( colors );
				(*it).quantize();
			}
			 if (height != 0 || width != 0)
				(*it).sample(Geometry(width,height));
			w = (*it).columns();
			h = (*it).rows();
			/*
			if ((*it).depth() > 8) {
				esyslog("ERROR: text2skin: More than 8bpp images are not supported");
				return false;
			}
			*/
			bmp = new cBitmap(w, h, std::min(size_t((*it).depth()), size_t(8)));
			//Dprintf("this image has %d colors\n", (*it).totalColors());

			const PixelPacket *pix = (*it).getConstPixels(0, 0, w, h);
			for (int iy = 0; iy < h; ++iy) {
				for (int ix = 0; ix < w; ++ix) {
					tColor col = (~int(pix->opacity * 255 / MaxRGB) << 24) | (int(pix->red * 255 / MaxRGB) << 16) | (int(pix->green * 255 / MaxRGB) << 8) | int(pix->blue * 255 / MaxRGB);
					bmp->DrawPixel(ix, iy, col);
					++pix;
				}
			}
			mBitmaps.push_back(bmp);
		}
	} catch (Exception &e) {
		if (!Quiet)
			esyslog("ERROR: text2skin: Couldn't load %s: %s", Filename, e.what());
		delete bmp;
		return false;
	} catch (...) {
		if (!Quiet)
			esyslog("ERROR: text2skin: Couldn't load %s: Unknown exception caught", Filename);
		delete bmp;
		return false;
	}
	return true;

#elif defined(HAVE_IMLIB2)
	Imlib_Image image;
        unsigned char * outputImage = NULL;
	unsigned int * outputPalette = NULL;
	cQuantizeWu* quantizer = new cQuantizeWu();
	cBitmap *bmp = NULL;
	image = imlib_load_image(Filename);
	if (!image)
		return false;
	Imlib_Context ctx = imlib_context_new();
	imlib_context_push(ctx);
	if (height != 0 || width != 0){
		imlib_context_set_image(image);
		image = imlib_create_cropped_scaled_image(0,0,imlib_image_get_width(), imlib_image_get_height() ,width , height);
	}
	imlib_context_set_image(image);
	bmp = new cBitmap(imlib_image_get_width(), imlib_image_get_height(), 8);
	uint8_t *data = (uint8_t*)imlib_image_get_data_for_reading_only();
	if ( colors != 0 ){
        	quantizer->Quantize(data, imlib_image_get_width()* imlib_image_get_height(), colors);
		outputImage = quantizer->OutputImage();
		outputPalette = quantizer->OutputPalette();
	}
	int pos = 0;
	for (int y = 0; y < bmp->Height(); ++y) {
		for (int x = 0; x < bmp->Width(); ++x) {
			if ( colors != 0 ){
				bmp->DrawPixel(x, y ,  outputPalette[outputImage[y * bmp->Width() + x]] | 0xFF000000 );
			}else{
				tColor col = (data[pos + 3] << 24) | (data[pos + 2] << 16) | (data[pos + 1] << 8) | data[pos + 0];
				bmp->DrawPixel(x, y, col);
				pos += 4;
			}
		}
	}

	imlib_free_image();
	imlib_context_free(ctx);
	mBitmaps.push_back(bmp);
	delete(quantizer);
	return true;

#else /* Not built with external image library */
	if (!Quiet)
		esyslog("ERROR: text2skin: unknown file format for %s", Filename);
	return false;

#endif
}
