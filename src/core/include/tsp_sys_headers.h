/*

$Header: /sources/tsp/tsp/src/core/include/tsp_sys_headers.h,v 1.25 2008/03/24 23:56:21 deweerdt Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2002 Yves DUFRENNE, Stephane GALLES, Eric NOULARD and Robert PAGNOT 

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
Maintainer : tsp@astrium.eads.net
Component : Consumer / Provider

-----------------------------------------------------------------------

Purpose   : 
-----------------------------------------------------------------------
 */

#ifndef __TSP_SYS_HEADERS_H
#define __TSP_SYS_HEADERS_H

#ifdef __FreeBSD__
#include <rpc/rpc.h>
#endif /* __FreeBSD__ */

#ifdef __OpenBSD__
#include <rpc/rpc.h>
/* OpenBSD's rpcgen has timeout support and declares _rpcsvcdirty as extern */
int _rpcsvcdirty;
#endif /* __OpenBSD__ */

/* FIXME using POSIX_SOURCE on SOLARIS gives me very strange compilation trouble (Erk) */
#if !defined(_POSIX_SOURCE) && !(defined(sun) || defined(__sun) || defined(__rtems__))
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#ifndef WIN32
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(sun) || defined(__sun)
#include <alloca.h>
#include <rpc/rpc.h>
#endif

#include <string.h>

#if  !defined(WIN32) && !defined(__rtems__)
#include <strings.h>
#endif

#include <errno.h>

#ifndef WIN32 /* conflict error C2011 under VC++, already include by <rpc/rpc.h> */
#include <netdb.h>
#endif


/* 
 * Used for gprof to work for linux multi-thread programs.
 * Do not use this with others targets 
 */
#ifdef MT_GPROF
#include "tsp_mtgprof.h"
#define pthread_create gprof_pthread_create
#endif

/* OSF1 / Alpha */
#if defined (__osf__) && defined (__alpha)
# define strtoll strtol
# define strtoull strtoul
#endif /* OSF1 / Alpha */

#include <pthread.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/param.h>
#endif

#ifdef VXWORKS
/* VXWORKS part */
# include <hostLib.h>
# include <sockLib.h>
# include <logLib.h>
# include <tickLib.h>
# include <taskLib.h>
/* posix part */
# include <libgen.h>
# include <strings.h>
#endif /* VXWORKS */

#endif
