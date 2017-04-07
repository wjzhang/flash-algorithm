/* CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../../FlashOS.H"        // FlashOS Structures

struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STM32F071 128 KB Flash",    // Device Name
   ONCHIP,                     // Device Type
   0x08000000,                 // Flash start address
   0x00020000,                 // Flash total size (128KB )
   2048,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   1000,                        // Program Page Timeout 100 mSec
   10000,                       // Erase Sector Timeout 3000 mSec
	
							   // Specify Size and Address of Sectors
  0x00000800, 0x00000000,     // Sector Size  2 KB (64 Sectors)
  SECTOR_END                  // Marks end of sector table
};
