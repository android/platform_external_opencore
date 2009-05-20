/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.5.0   March 2, 2001
*                                R99   Version 3.2.0
*                                REL-4 Version 4.0.0
*
********************************************************************************
*
*      File             : d_homing_tab.h
*      Purpose          : definitions for decoder homing frames
*
*      $Id $
*
********************************************************************************
*/

#ifndef __d_homing_tab_defined
#define __d_homing_tab_defined

#include "typedef.h"
#include "mode.h"

#include "bitno_tab.h"

extern const Word16 dhf_MR475[PRMNO_MR475];
extern const Word16 dhf_MR515[PRMNO_MR515];
extern const Word16 dhf_MR59[PRMNO_MR59];
extern const Word16 dhf_MR67[PRMNO_MR67];
extern const Word16 dhf_MR74[PRMNO_MR74];
extern const Word16 dhf_MR795[PRMNO_MR795];
extern const Word16 dhf_MR102[PRMNO_MR102];
extern const Word16 dhf_MR122[PRMNO_MR122];

/* overall table with the parameters of the
   decoder homing frames for all modes */
// extra const needed for symbian
extern const Word16* const dhf[];

#endif
