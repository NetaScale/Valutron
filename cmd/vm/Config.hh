#ifndef CONFIG_HH_
#define CONFIG_HH_

#define VT_GC_MPS 0 /**< use Ravenbrook Memory Pool System */
#define VT_GC_BOEHM 1 /**< use Boehm-Demers-Weiser LibGC */
#define VT_GC_MALLOC 2 /**< use no GC; uses malloc and never frees */

#define VT_GC VT_GC_BOEHM

#endif /* CONFIG_HH_ */
