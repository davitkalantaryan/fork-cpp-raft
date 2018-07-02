// 
// file:		lists.hpp
// created on:	2018 Jun 02
// created by:	D. Kalantaryan (davit.kalantaryan@desy.de)
//

#ifndef __common_lists_hpp__
#define __common_lists_hpp__

#include <stddef.h>

namespace common {

// By special we assume that
// template argument Type has is one of this
// a) pointer
// b) or class, where operator->() is overloaded
//
// if we have Type aData;, then aData->prev is valid, and aData->next is valid
// 
// usually better to use the class common::List, that works for any type

template <typename Type>
class ListSpecial
{
public:
	ListSpecial();
	virtual ~ListSpecial();

	void  AddDataRaw(Type newData);
	Type  RemoveDataRaw(Type dataToRemove);

	Type  first()const {return m_first;}
	int   count()const {return m_nCount;}

	void  MoveContentToOtherList(ListSpecial* pOther);
	void  MoveItemToOtherList(ListSpecial* pOther, Type item);

protected:
	Type	m_first, m_last;
	int		m_nCount;
};

template <typename Type>
class List;

namespace listN {

template <typename ItemType>
struct ListItem {
	ListItem *prev, *next;
	ItemType data;
	/*---------------------*/
	friend class List<ItemType>;
private:
	ListItem(const ItemType& a_data):data(a_data){}
	~ListItem() {}
};

} // namespace listN {


template <typename Type>
class List : public ListSpecial<common::listN::ListItem<Type>* >
{
public:
	virtual ~List();

	common::listN::ListItem<Type>* AddData(const Type& newData);
	common::listN::ListItem<Type>* RemoveData(common::listN::ListItem<Type>* itemToRemove);

};


} // namespace common {

#include "impl.lists.hpp"


#endif  // #ifndef __common_lists_hpp__
