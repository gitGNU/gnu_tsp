/*!  \file

$Header: /sources/tsp/tsp/src/core/tests/util/Attic/unit.h,v 1.2 2002/12/18 16:27:49 tntdev Exp $

-----------------------------------------------------------------------

libUTIL - core components to read and write res files

Copyright (c) 2002 Marc LE ROY 

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
Component : libUTIL

-----------------------------------------------------------------------

Purpose   :

-----------------------------------------------------------------------
 */


static	char	*unit_h_sccsid={"@(#)unit.h\t1.3\t00/09/29\tMATRA PROJET"};

struct dim {
	int	t[6];
	};

struct sunit {
	struct dim d;
	double conv;
	} ;

struct sunit	si_unit (char *unit);
int		add_dim (struct dim *a, struct dim *b, int ordr);
int		cmp_unit (struct dim *a, struct dim *b);
