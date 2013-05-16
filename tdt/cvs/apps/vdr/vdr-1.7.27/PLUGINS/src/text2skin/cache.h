//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_CACHE_HPP
#define VDR_TEXT2SKIN_CACHE_HPP

#include <cstdlib>
#include <map>
#include <vector>
#include <string>

template<class key_type, class data_type>
class cxCache {
private:
	typedef std::vector<key_type> usage_list;
	typedef typename usage_list::iterator usage_iterator;
	typedef std::map<key_type,data_type> item_map;
	typedef typename item_map::iterator item_iterator;

	item_map   mItems;
	usage_list mUsage;
	uint       mMaxItems;

protected:
	virtual void DeleteObject(const key_type &Key, data_type &Data) = 0;
	virtual void ResetObject(data_type &Data) = 0;

public:
	cxCache(uint MaxItems);
	virtual ~cxCache();

	void Reset(void);
	void Flush(void);
	bool Contains(const key_type &Key);
	data_type &operator[](const key_type &Key);
	uint Count(void) { return mUsage.size(); }
	void SetMaxItems(uint MaxItems) { mMaxItems = MaxItems; }
};

template<class key_type, class data_type>
inline bool cxCache<key_type, data_type>::Contains(const key_type &Key)
{
	return mItems.find(Key) != mItems.end();
}

template<class key_type, class data_type>
cxCache<key_type, data_type>::cxCache(uint MaxItems)
{
	mMaxItems = MaxItems;
}

template<class key_type, class data_type>
cxCache<key_type, data_type>::~cxCache()
{
}

template<class key_type, class data_type>
void cxCache<key_type, data_type>::Flush(void)
{
	item_iterator it = mItems.begin();
	for (; it != mItems.end(); ++it)
		DeleteObject(it->first, it->second);

	mUsage.clear();
	mItems.clear();
}

template<class key_type, class data_type>
void cxCache<key_type, data_type>::Reset(void)
{
	item_iterator it = mItems.begin();
	for (; it != mItems.end(); ++it)
		ResetObject(it->second);
}

template<class key_type, class data_type>
data_type &cxCache<key_type, data_type>::operator[](const key_type &Key)
{
	item_iterator it = mItems.find(Key);
	if (it != mItems.end()) {
		usage_iterator ut = mUsage.begin();
		for (; ut != mUsage.end(); ++ut) {
			if (*ut == Key) {
				mUsage.erase(ut);
				break;
			}
		}
		mUsage.push_back(Key);
		return it->second;
	}

	while (mUsage.size() >= mMaxItems) {
		item_iterator it = mItems.find(*mUsage.begin());
		DeleteObject(it->first, it->second);
		mUsage.erase(mUsage.begin());
		mItems.erase(it);
	}

	it = mItems.insert(typename item_map::value_type(Key, data_type())).first;
	return it->second;
}

#endif // VDR_TEXT2SKIN_CACHE_HPP
