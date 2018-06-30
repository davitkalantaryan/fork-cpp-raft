//
// file:		impl.lists.hpp
// created on:	2018 Jun 02
// created by:	D. Kalantaryan
//

#ifndef __common_impl_lists_hpp__
#define __common_impl_lists_hpp__


#ifndef __common_lists_hpp__
#error do not include this file directly
#include "lists.hpp"
#endif

#include <stdlib.h>
#ifdef _WIN32
#include <malloc.h>
#endif

template <typename Type>
common::ListSpecial<Type>::ListSpecial()
	:
	m_first((Type)0),
	m_last((Type)0),
	m_nCount(0)
{
}


template <typename Type>
common::ListSpecial<Type>::~ListSpecial()
{
}


template <typename Type>
void common::ListSpecial<Type>::AddData(Type a_newData)
{
	if(!m_first){a_newData->prev=(Type)0;m_first=a_newData;}
	else{m_last->next= a_newData;}
	a_newData->prev = m_last;
	a_newData->next = (Type)0;
	m_last = a_newData;
	++m_nCount;
}


template <typename Type>
Type common::ListSpecial<Type>::RemoveData(Type a_dataToRemove)
{
	if(a_dataToRemove->prev){ a_dataToRemove->prev->next= a_dataToRemove->next;}
	if(a_dataToRemove->next){ a_dataToRemove->next->prev= a_dataToRemove->prev;}
	if(a_dataToRemove == m_first){m_first=((Type)a_dataToRemove->next);}
	if(a_dataToRemove == m_last){m_last= ((Type)a_dataToRemove->prev);}
	--m_nCount;
	return (Type)a_dataToRemove->next;
}


/*////////////////////////////////////////////////////////////////////////////////*/
template <typename Type>
common::List<Type>::~List()
{
}


template <typename Type>
common::listN::ListItem<Type>* common::List<Type>::AddData(const Type& a_newData)
{
	//common::listN::ListItem<Type>* pNewItem = (common::listN::ListItem<Type>*)malloc(sizeof(common::listN::ListItem<Type>));
	common::listN::ListItem<Type>* pNewItem = new common::listN::ListItem<Type>(a_newData);
	if(!pNewItem){return NULL;}
	m_listSp.AddData(pNewItem);
	return pNewItem;
}


template <typename Type>
common::listN::ListItem<Type>* common::List<Type>::RemoveData(common::listN::ListItem<Type>* a_itemToRemove)
{
	common::listN::ListItem<Type>* pReturn = m_listSp.RemoveData(a_itemToRemove);
	delete a_itemToRemove;
	return pReturn;
}

#endif  // #ifndef __common_impl_lists_hpp__
