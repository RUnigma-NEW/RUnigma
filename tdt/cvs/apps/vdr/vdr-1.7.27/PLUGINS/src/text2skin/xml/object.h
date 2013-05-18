//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_XML_OBJECT_H
#define VDR_TEXT2SKIN_XML_OBJECT_H

#include "xml/string.h"
#include "xml/function.h"
#include <vdr/osd.h>
#include <vector>
#include <string>
#include <map>

class cxDisplay;
class cxSkin;

struct txPoint {
	int x, y;
	txPoint(int _x = 0, int _y = 0) { x = _x; y = _y; }
};

struct txSize {
	int w, h;
	txSize(int _w = 0, int _h = 0) { w = _w; h = _h; }
};

struct txWindow {
	txPoint pos1;
	txPoint pos2;
	int bpp;
	txWindow(int _x1 = 0, int _y1 = 0, int _x2 = -1, int _y2 = -1, int _bpp=4):
			pos1(_x1, _y2), pos2(_x2, _y2), bpp(_bpp) {}
};

// state information for marquee, blink, scroll
struct txState {
	bool        scrolling;
	int         offset;
	int         direction;
	uint        nexttime;
	std::string text;
	txState(void): scrolling(false), offset(0), direction(1), nexttime(0) {}
};

class cxObject;

class cxRefresh {
	friend bool xEndElem(const std::string &name);

public:
	enum eRefreshType {
		all,            // complete redraw of the screen
		timeout,        // redraw due to a timeout
		//message,      // a message was set or removed
		update,         // update of the osd elements
		scroll,         // a scroll event
		list,           // list items or the current item have changed
	};

	cxRefresh(cxObject *Object);
	~cxRefresh();
	bool           Dirty(uint dirty, uint &updatein, bool force = false, uint now = 0);
	bool           Full(void)      const { return mFull; }
	uint           Type(void)      const { return mRefreshType; }
	bool           Parse(const std::string &Text);
	bool           ParseChanged(const std::string &Text);
	cxRefresh     &operator=(const cxRefresh &b);

private:
	uint           mRefreshType;
	cxType         mLastEval;
	cxType         mEval;
	cxString      *mText;
	cxString      *mChanged;
	cxObject      *mObject;
	bool           mForce, mFull;
};

class cxObjects;

class cxObject {
	friend class cText2SkinRender;
	friend bool xStartElem(const std::string &name, std::map<std::string,std::string> &attrs);
	friend bool xCharData(const std::string &text);
	friend bool xEndElem(const std::string &name);

	/* Skin Editor */
	friend class VSkinnerItem;

public:
	enum eType {
		image,
		text,
		marquee,
		blink,
		rectangle,
		ellipse,
		slope,
		progress,
		scrolltext,
		scrollbar,
		block,
		list,
		item,
#define __COUNT_OBJECT__ (item + 1)
	};

private:
	cxDisplay     *mDisplay;
	cxSkin        *mSkin;
	eType          mType;
	txPoint        mPos1;
	txPoint        mPos2;
	txSize         mVirtSize;
	int            mAlpha;
	int            mColors;
	int            mArc;
	std::string    mFg;
	std::string    mBg;
	std::string    mBl;
	std::string    mMask;
	std::string    mMark;
	std::string    mActive;
	std::string    mKeep;
	cxString       mPath;
	cxString       mText;
	eTextAlignment mAlign;
	cxFunction    *mCondition;
	cxString       mCurrent;
	cxString       mTotal;
	std::string    mFontFace;
	int            mFontSize;
	int            mFontWidth;
	uint           mDelay;
	uint           mIndex;
	cxRefresh      mRefresh;
	cxObjects     *mObjects; // used for block objects such as <list>

	// state information for marquee, blink, scroll
	uint           mListIndex;
	typedef std::map<uint,txState> tStates;
	tStates        mStates;

public:
	cxObject(cxDisplay *parent);
	cxObject(const cxObject &Src);
	~cxObject();

	bool ParseType     (const std::string &Text);
	bool ParseCondition(const std::string &Text);
	bool ParseAlignment(const std::string &Text);
	bool ParseFontFace (const std::string &Text);

	void               SetListIndex(uint Index, int Tab);

	eType              Type(void)            const { return mType; }
	cxFunction        *Condition(void)       const { return mCondition; }
	int                Alpha(void)           const { return mAlpha; }
	int                Colors(void)          const { return mColors; }
	eTextAlignment     Align(void)           const { return mAlign; }
	int                Arc(void)             const { return mArc; }
	std::string        Path(void)            const { return mPath.Evaluate(); }
	std::string        Text(void)            const { return mText.Evaluate(); }
	int                Current(void)         const { return mCurrent.Evaluate(); }
	int                Total(void)           const { return mTotal.Evaluate(); }
	uint               Delay(void)           const { return mDelay; }
	uint               Index(void)           const { return mIndex; }
	cxDisplay         *Display(void)         const { return mDisplay; }
	cxSkin            *Skin(void)            const { return mSkin; }
	txState           &State(void)                 { return mStates[mListIndex]; }

	const std::string &TypeName(void)        const;
	txPoint            Pos(const txPoint &BaseOffset = txPoint(-1, -1), const txSize &BaseSize = txSize(-1, -1),
	                       const txSize &VirtSize = txSize(-1, -1)) const;
	txSize             Size(const txPoint &BaseOffset = txPoint(-1, -1), const txSize &BaseSize = txSize(-1, -1),
	                        const txSize &VirtSize = txSize(-1, -1)) const;
	const cFont       *Font(void)            const;
	const tColor      *Fg(void)              const;
	const tColor      *Bg(void)              const;
	const tColor      *Bl(void)              const;
	const tColor      *Mask(void)            const;
	const tColor      *Mark(void)            const;
	const tColor      *Active(void)          const;
	const tColor      *Keep(void)            const;

	uint               Objects(void)         const;
	cxObject          *GetObject(uint Index) const;
};

class cxObjects: public std::vector<cxObject*> {
public:
	cxObjects(void);
	~cxObjects();
};

// recursive dependancy
inline uint cxObject::Objects(void) const
{
	return mObjects ? mObjects->size() : 0;
}

inline cxObject *cxObject::GetObject(uint Index) const
{
	return mObjects ? (*mObjects)[Index] : NULL;
}

#endif // VDR_TEXT2SKIN_XML_OBJECT_H
