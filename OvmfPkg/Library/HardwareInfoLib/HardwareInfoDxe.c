/*/@file
  Hardware info parsing functions.
  Binary data is expected as a consecutive series of header - object pairs.
  Complete library providing list-like interface to dynamically manipulate
  hardware info objects and parsing from a generic blob.

  Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <string.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>

#include <Library/HardwareInfoLib.h>

EFI_STATUS
CreateHardwareInfoList (
  IN  UINT8               *Blob,
  IN  UINTN               BlobSize,
  IN  HARDWARE_INFO_TYPE  TypeFilter,
  OUT LIST_ENTRY          *ListHead
  )
{
  UINT8          *Index;
  HARDWARE_INFO  *HwComponent;

  if ((Blob == NULL) || (BlobSize <= 0) ||
      (ListHead == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Index = Blob;
  while (Index < (Blob + BlobSize)) {
    HwComponent = AllocateZeroPool (sizeof (HARDWARE_INFO));

    if (HwComponent == NULL) {
      goto FailedAllocate;
    }

    HwComponent->Header.Type.Uint64 = *((UINT64 *)Index);
    Index                          += sizeof (HwComponent->Header.Type);
    HwComponent->Header.Size        = *((UINT64 *)(Index));
    Index                          += sizeof (HwComponent->Header.Size);

    //
    // Check if optional TypeFilter is set, skip if the current
    // object is of a different type and release the partially
    // allocated object
    //
    if ((TypeFilter != HardwareInfoTypeUndefined) &&
        (HwComponent->Header.Type.Value != TypeFilter))
    {
      FreePool (HwComponent);
      Index += HwComponent->Header.Size;
      continue;
    }

    HwComponent->Data.Raw = AllocateZeroPool (HwComponent->Header.Size);
    if (HwComponent->Data.Raw == NULL) {
      goto FreeResources;
    }

    CopyMem (HwComponent->Data.Raw, Index, HwComponent->Header.Size);
    Index += HwComponent->Header.Size;

    InsertTailList (ListHead, &HwComponent->Link);
  }

  return EFI_SUCCESS;

FreeResources:
  //
  // Clean the resources allocated in the incomplete cycle
  //
  FreePool (HwComponent);

FailedAllocate:
  DEBUG ((
    EFI_D_ERROR,
    "%a: Failed to allocate memory for hardware info\n",
    __FUNCTION__
    ));

  return EFI_OUT_OF_RESOURCES;
}

VOID
FreeHardwareInfoList (
  IN OUT  LIST_ENTRY  *ListHead
  )
{
  LIST_ENTRY     *CurrentLink;
  HARDWARE_INFO  *HwComponent;

  if (IsListEmpty (ListHead)) {
    return;
  }

  CurrentLink = ListHead->ForwardLink;
  while (CurrentLink != NULL && CurrentLink != ListHead) {
    HwComponent = HARDWARE_INFO_FROM_LINK (CurrentLink);

    //
    // Remove item from list before invalidating the pointers
    //
    CurrentLink = RemoveEntryList (CurrentLink);

    FreePool (HwComponent->Data.Raw);
    FreePool (HwComponent);
  }
}

/**
  Validates if the specified Node has a valid data size and is of
  specified type.
  The data size can be less or equal to the provided type size to be
  regarded as valid and thus accessible with the typed pointer.

  For future compatibility the size is allowed to be smaller so that
  different versions interpret fields differently and, particularly,
  have smaller data structures. However, it cannot be larger than the
  type size to avoid accessing memory out of bounds.

  @param[in]  Node      Hardware Info node to be validated
  @param[in]  TypeSize  Size (in bytes) of the data type intended to be
                        used to dereference the data.
  @retval TRUE  Node is valid and can be accessed
  @retval FALSE Node is not valid
/*/
STATIC
BOOLEAN
IsHardwareInfoNodeValidByType (
  IN  LIST_ENTRY          *ListHead,
  IN  LIST_ENTRY          *Link,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  HARDWARE_INFO  *HwComponent;

  if (IsNull (ListHead, Link)) {
    return FALSE;
  }

  HwComponent = HARDWARE_INFO_FROM_LINK (Link);

  //
  // Verify if the node type is the specified one and the size of
  // the data allocated to the node is greater than the size of
  // the type intended to dereference it in order to avoid access
  // to memory out of bondaries.
  //
  if ((HwComponent->Header.Type.Value == Type) &&
      (HwComponent->Header.Size >= TypeSize))
  {
    return TRUE;
  }

  return FALSE;
}

UINTN
GetHardwareInfoCountByType (
  IN  LIST_ENTRY          *ListHead,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  UINTN       Count;
  LIST_ENTRY  *Link;

  Count = 0;
  for (Link = GetFirstHardwareInfoByType (ListHead, Type, TypeSize);
       !IsNull (ListHead, Link);
       Link = GetNextHardwareInfoByType (ListHead, Link, Type, TypeSize))
  {
    if (IsHardwareInfoNodeValidByType (ListHead, Link, Type, TypeSize)) {
      Count++;
    }
  }

  return Count;
}

LIST_ENTRY *
GetFirstHardwareInfoByType (
  IN  LIST_ENTRY          *ListHead,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  LIST_ENTRY  *Link;

  if (IsListEmpty (ListHead)) {
    return ListHead;
  }

  Link = GetFirstNode (ListHead);

  if (IsHardwareInfoNodeValidByType (ListHead, Link, Type, TypeSize)) {
    return Link;
  }

  return GetNextHardwareInfoByType (ListHead, Link, Type, TypeSize);
}

LIST_ENTRY *
GetNextHardwareInfoByType (
  IN  LIST_ENTRY          *ListHead,
  IN  LIST_ENTRY          *Node,
  IN  HARDWARE_INFO_TYPE  Type,
  IN  UINTN               TypeSize
  )
{
  LIST_ENTRY  *Link;

  Link = GetNextNode (ListHead, Node);

  while (!IsNull (ListHead, Link)) {
    if (IsHardwareInfoNodeValidByType (ListHead, Link, Type, TypeSize)) {
      //
      // Found a node of specified type and with valid size. Break and
      // return the found node.
      //
      break;
    }

    Link = GetNextNode (ListHead, Link);
  }

  return Link;
}

BOOLEAN
EndOfHardwareInfoList (
  IN  LIST_ENTRY  *ListHead,
  IN  LIST_ENTRY  *Node
  )
{
  return IsNull (ListHead, Node);
}
