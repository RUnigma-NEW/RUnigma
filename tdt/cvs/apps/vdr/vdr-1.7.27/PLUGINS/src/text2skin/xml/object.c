//								-*- c++ -*-

#include "xml/object.h"
#include "render.h"
#include "font.h"

static const std::string ObjectNames[] =
	{ "image", "text", "marquee", "blink", "rectangle", "ellipse", "slope", "progress",
	  "scrolltext", "scrollbar", "block", "list", "item" };

cxObject::cxObject(cxDisplay *Parent):
		mDisplay(Parent),
		mSkin(Parent->Skin()),
		mType((eType)__COUNT_OBJECT__),
		mPos1(0, 0),
		mPos2(-1, -1),
		mVirtSize(-1, -1),
		mAlpha(255),
		mColors(0),
		mArc(0),
		mPath(this, false),
		mText(this, true),
		mAlign(taDefault),
		mCondition(NULL),
		mCurrent(this, false),
		mTotal(this, false),
		mFontFace("Osd"),
		mFontSize(0),
		mFontWidth(0),
		mDelay(150),
		mIndex(0),
		mRefresh(this),
		mObjects(NULL),
		mListIndex(0)
{
}

cxObject::cxObject(const cxObject &Src):
		mDisplay(Src.mDisplay),
		mSkin(Src.mSkin),
		mType(Src.mType),
		mPos1(Src.mPos1),
		mPos2(Src.mPos2),
		mVirtSize(Src.mVirtSize),
		mAlpha(Src.mAlpha),
		mColors(Src.mColors),
		mArc(Src.mArc),
		mFg(Src.mFg),
		mBg(Src.mBg),
		mBl(Src.mBl),
		mMask(Src.mMask),
		mMark(Src.mMark),
		mActive(Src.mActive),
		mKeep(Src.mKeep),
		mPath(Src.mPath),
		mText(Src.mText),
		mAlign(Src.mAlign),
		mCondition(NULL),
		mCurrent(Src.mCurrent),
		mTotal(Src.mTotal),
		mFontFace(Src.mFontFace),
		mFontSize(Src.mFontSize),
		mFontWidth(Src.mFontWidth),
		mDelay(Src.mDelay),
		mRefresh(Src.mRefresh),
		mObjects(NULL),
		mListIndex(Src.mListIndex)
{
	if (Src.mCondition)
		mCondition = new cxFunction(*Src.mCondition);
	if (Src.mObjects)
		mObjects = new cxObjects(*Src.mObjects);
}

cxObject::~cxObject()
{
	delete mCondition;
	delete mObjects;
}

bool cxObject::ParseType(const std::string &Text)
{
	for (int i = 0; i < (int)__COUNT_OBJECT__; ++i) {
		if (ObjectNames[i] == Text) {
			mType = (eType)i;
			return true;
		}
	}
	return false;
}

bool cxObject::ParseCondition(const std::string &Text)
{
	cxFunction *result = new cxFunction(this);
	if (result->Parse(Text)) {
		delete mCondition;
		mCondition = result;
		return true;
	}
	return false;
}

bool cxObject::ParseAlignment(const std::string &Text)
{
	if      (Text == "center") mAlign = (eTextAlignment)(taTop | taCenter);
	else if (Text == "right")  mAlign = (eTextAlignment)(taTop | taRight);
	else if (Text == "left")   mAlign = (eTextAlignment)(taTop | taLeft);
	else
		return false;
	return true;
}

bool cxObject::ParseFontFace(const std::string &Text)
{
	int size = 0, width = 0, pos;
	std::string face = Text;
	if ((pos = face.find('@')) != -1) {
		std::string s = face.substr(pos + 1);
		const char *p = s.c_str();
		char *end;
		size = strtol(p, &end, 10);
		if (*end == ',')
			width = strtol(end + 1, NULL, 10);

		face.erase(pos);
	}

	mFontFace  = face;
	mFontSize  = size;
	mFontWidth = width;
	return true;
}

void cxObject::SetListIndex(uint Index, int Tab)
{
	Tab = (Tab >= 0 ? Tab : -1);
	mListIndex = 1 + Index * cSkinDisplayMenu::MaxTabs + Tab;
	mText.SetListIndex(Index, Tab);
	mPath.SetListIndex(Index, Tab);
	if (mCondition != NULL)
		mCondition->SetListIndex(Index, Tab);
}

const std::string &cxObject::TypeName(void) const
{
	return ObjectNames[mType];
}

const cFont *cxObject::Font(void) const
{
	const cFont *font;

	if ((font = cText2SkinFont::Load(mFontFace, mFontSize)) != NULL)
		return font;

	return cFont::GetFont(fontOsd);
}

txPoint cxObject::Pos(const txPoint &BaseOffset, const txSize &BaseSize, const txSize &VirtSize) const
{
	txPoint bOffset = BaseOffset.x < 0 ? mSkin->BaseOffset() : BaseOffset;
	txSize  bSize   = BaseSize.w   < 0 ? mSkin->BaseSize()   : BaseSize;

	double scale_x = VirtSize.w > 0 ? (double)BaseSize.w / VirtSize.w : 1.0,
	       scale_y = VirtSize.h > 0 ? (double)BaseSize.h / VirtSize.h : 1.0;

	int x1 = mPos1.x < 0 ? (int)((mPos1.x + 1) * scale_x - 1) : (int)(mPos1.x * scale_x);
	int y1 = mPos1.y < 0 ? (int)((mPos1.y + 1) * scale_x - 1) : (int)(mPos1.y * scale_y);

	return txPoint(bOffset.x + (x1 < 0 ? bSize.w + x1 : x1),
	               bOffset.y + (y1 < 0 ? bSize.h + y1 : y1));
}

txSize cxObject::Size(const txPoint &BaseOffset, const txSize &BaseSize, const txSize &VirtSize) const
{
	//txPoint bOffset = BaseOffset.x < 0 ? mSkin->BaseOffset() : BaseOffset;
	txSize  bSize   = BaseSize.w   < 0 ? mSkin->BaseSize()   : BaseSize;

	double scale_x = VirtSize.w > 0 ? (double)BaseSize.w / VirtSize.w : 1.0,
	       scale_y = VirtSize.h > 0 ? (double)BaseSize.h / VirtSize.h : 1.0;

	int x1 = mPos1.x < 0 ? (int)((mPos1.x + 1) * scale_x - 1) : (int)(mPos1.x * scale_x);
	int y1 = mPos1.y < 0 ? (int)((mPos1.y + 1) * scale_x - 1) : (int)(mPos1.y * scale_y);
	int x2 = mPos2.x < 0 ? (int)((mPos2.x + 1) * scale_x - 1) : (int)(mPos2.x * scale_x);
	int y2 = mPos2.y < 0 ? (int)((mPos2.y + 1) * scale_x - 1) : (int)(mPos2.y * scale_y);

	txPoint p1(x1 < 0 ? bSize.w + x1 : x1,
	           y1 < 0 ? bSize.h + y1 : y1);
	txPoint p2(x2 < 0 ? bSize.w + x2 : x2,
	           y2 < 0 ? bSize.h + y2 : y2);

	return txSize(p2.x - p1.x + 1, p2.y - p1.y + 1);
}

const tColor *cxObject::Fg(void) const
{
	static tColor Fg;
	return cText2SkinRender::ItemColor(mFg, Fg) ? &Fg : NULL;
}

const tColor *cxObject::Bg(void) const
{
	static tColor Bg;
	return cText2SkinRender::ItemColor(mBg, Bg) ? &Bg : NULL;
}

const tColor *cxObject::Bl(void) const
{
	static tColor Bl;
	return cText2SkinRender::ItemColor(mBl, Bl) ? &Bl : NULL;
}

const tColor *cxObject::Mask(void) const
{
	static tColor Mask;
	return cText2SkinRender::ItemColor(mMask, Mask) ? &Mask : NULL;
}

const tColor *cxObject::Mark(void) const
{
	static tColor Mark;
	return cText2SkinRender::ItemColor(mMark, Mark) ? &Mark : NULL;
}

const tColor *cxObject::Active(void) const
{
	static tColor Active;
	return cText2SkinRender::ItemColor(mActive, Active) ? &Active : NULL;
}

const tColor *cxObject::Keep(void) const
{
	static tColor Keep;
	return cText2SkinRender::ItemColor(mKeep, Keep) ? &Keep : NULL;
}

cxObjects::cxObjects(void)
{
}

cxObjects::~cxObjects()
{
	for (uint i = 0; i < size(); ++i)
		delete operator[](i);
}


///////////////////////////////////////////////////////////////////////////////
// ---------- class cxRefresh ---------------------------------------------- //

cxRefresh::cxRefresh(cxObject *Object):
	mRefreshType(0xFF),
	mText(NULL),
	mChanged(NULL),
	mObject(Object),
	mForce(true),
	mFull(true)
{
}

cxRefresh::~cxRefresh()
{
	delete mText;
}

bool cxRefresh::Dirty(uint dirty, uint &updatein, bool force, uint now)
{
	// check if the timeout of the object has expired
	uint nexttime = mObject->State().nexttime;

	bool to = force || mForce ||
	          mObject->Type() == cxObject::block || mObject->Type() == cxObject::list;
	bool changed = force || mForce;

	if (now > 0 && nexttime > 0) {
		// timeout was set
		if (now >= nexttime)
			// timeout has expired
			to = true;
		else {
			// time left -> set new update interval
			uint nextin = nexttime - now;
			if (updatein == 0 || nextin < updatein)
				updatein = nextin;
		}
	}

	// Object has changed since last redraw
	if (mChanged != NULL) {
		mEval = mChanged->Evaluate();
		if (mEval != mLastEval)
			changed = true;
	}

	// refresh
	if ((mRefreshType & dirty & ~(1<<timeout) & ~(1<<update))) {
		if (changed)
			mLastEval = mEval;
		return true;
	}

	// timeout
	if ((mRefreshType & dirty & (1<<timeout)) && to) {
		if (changed)
			mLastEval = mEval;
		return true;
	}

	// update
	if ((mRefreshType & dirty & (1<<update)) && changed) {
		mLastEval = mEval;
		return true;
	}

	return false;
}

bool cxRefresh::Parse(const std::string &Text)
{
	uint refresh = 0;
	bool force = false, full = false;

	if (Text.find("all") != std::string::npos)
		refresh |= (1<<all);

	if (Text.find("timeout") != std::string::npos)
		refresh |= (1<<timeout);

	if (Text.find("update") != std::string::npos)
		refresh |= (1<<update);

	//if (Text.find("message") != std::string::npos)
	//	refresh |= (1<<list);

	if (Text.find("list") != std::string::npos)
		refresh |= (1<<list);

	if (Text.find("scroll") != std::string::npos)
		refresh |= (1<<scroll);

	if (Text.find("always") != std::string::npos)
		refresh |= 0xFF;

	if (Text.find("full") != std::string::npos)
		full = true;

	if (Text.find("force") != std::string::npos)
		force = true;

	if (refresh == 0)
		return false;

	mForce = force;
	mFull = full;
	mRefreshType = refresh;

	return true;
}

bool cxRefresh::ParseChanged(const std::string &Text)
{
	if (mObject == NULL)
		return false;

	if (mText == NULL)
		mText = new cxString(mObject, false);

	if (mText->Parse(Text)) {
		mChanged = mText;
		return true;
	}

	return false;
}

cxRefresh &cxRefresh::operator=(const cxRefresh &a)
{
	mRefreshType = a.mRefreshType;
	mForce = a.mForce;
	mFull = a.mFull;
	return *this;
}
