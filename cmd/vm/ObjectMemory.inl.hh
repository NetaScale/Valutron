#ifndef OBJECTMEMORY_INL_HH_
#define OBJECTMEMORY_INL_HH_

#include "ObjectMemory.hh"

template <class T>
void
ObjectAllocator<T>::init(mps_pool_t amcPool, mps_pool_t amczPool)
{
	mps_res_t res = mps_ap_create_k(&m_objAP, amcPool, mps_args_none);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create allocation point!");

	res = mps_ap_create_k(&m_leafAp, amczPool, mps_args_none);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create allocation point!");
}

#endif /* OBJECTMEMORY_INL_HH_ */
