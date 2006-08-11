/*

$Header: /sources/tsp/tsp/src/util/libbb/bb_core_sysv.h,v 1.1.2.1 2006/08/11 08:49:47 deweerdt Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2006 Eric NOULARD, Frederik DEWEERDT

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-----------------------------------------------------------------------

Project   : TSP
Maintainer : tsp@astrium-space.com
Component : BlackBoard

-----------------------------------------------------------------------
 */
#ifndef _BB_CORE_SYSV_H_
#define _BB_CORE_SYSV_H_

#include <tsp_abs_types.h>
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
#ifdef _SEM_SEMUN_UNDEFINED
#undef _SEM_SEMUN_UNDEFINED
#endif
/*
 * Union a definir et utiliser
 * pour les appels 'semop' SysV.
 */
union semun {
  int val;                           /* value for SETVAL */
  struct semid_ds *buf;              /* buffer for IPC_STAT & IPC_SET */
  unsigned short int *array;         /* array for GETALL & SETALL */
  struct seminfo *__buf;             /* buffer for IPC_INFO */
};
#endif


#endif /* _BB_CORE_SYSV_H_ */
