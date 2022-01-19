/*/@file
  Hardware info parsing functions.
  Binary data is expected as a consecutive series of header - object pairs.
  Provides static Qemu fw-cfg wrappers as well as list-like interface to
  dynamically manipulate hardware info objects and parsing from a generic
  blob.

  Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Library/DebugLib.h>

#include <Library/HardwareInfoLib.h>

EFI_STATUS
CreateHardwareInfoList (
  IN  UINT8               *Blob,
  IN  UINTN               BlobSize,
  IN  HARDWARE_INFO_TYPE  TypeFilter,
  OUT LIST_ENTRY          *ListHead
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

VOID
FreeHardwareInfoList (
  IN OUT  LIST_ENTRY  *ListHead
  )
{
  ASSERT (FALSE);
}

UINTN
GetHardwareInfoCountByType (
  IN  LIST_ENTRY          *ListHead,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  ASSERT (FALSE);
  return 0;
}

LIST_ENTRY *
GetFirstHardwareInfoByType (
  IN  LIST_ENTRY          *ListHead,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  ASSERT (FALSE);
  return ListHead;
}

LIST_ENTRY *
GetNextHardwareInfoByType (
  IN  LIST_ENTRY          *ListHead,
  IN  LIST_ENTRY          *Node,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  ASSERT (FALSE);
  return ListHead;
}

BOOLEAN
EndOfHardwareInfoList (
  IN  LIST_ENTRY  *ListHead,
  IN  LIST_ENTRY  *Node
  )
{
  ASSERT (FALSE);
  return TRUE;
}
