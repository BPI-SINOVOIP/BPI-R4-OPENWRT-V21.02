/* adapter_global_cs_init.h
 *
 * Global Classification Control Initialization interface
 */

/*****************************************************************************
* Copyright (c) 2011-2020 by Rambus, Inc. and/or its subsidiaries.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef INCLUDE_GUARD_ADAPTER_GLOBAL_CS_INIT_H
#define INCLUDE_GUARD_ADAPTER_GLOBAL_CS_INIT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "basic_defs.h"


/*----------------------------------------------------------------------------
 *                           Adapter initialization
 *----------------------------------------------------------------------------
 */

bool
Adapter_Global_Cs_Init(void);

void
Adapter_Global_Cs_UnInit(void);

void
Adapter_Global_Cs_Report_Build_Params(void);


#endif /* Include Guard */


/* end of file adapter_global_cs_init.h */
