/*/@file
  Qemu fw-cfg wrappers for hardware info parsing.
  Provides an alternative to parse hardware information from a fw-cfg
  file without relying on dynamic memory allocations.

  Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Library/DebugLib.h>
#include <Library/QemuFwCfgLib.h>

#include <Library/HardwareInfoLib.h>

/**
  Update an optional pointer value if possible

  @param[out] DataSize     Pointer to variable to be updated
  @param[in]  Value        Value to set the pointed variable to.
**/
STATIC
VOID
UpdateDataSize (
  OUT UINTN  *DataSize,
  IN  UINTN  Value
  )
{
  if (DataSize == NULL) {
    return;
  }

  *DataSize = Value;
}

EFI_STATUS
QemuFwCfgReadNextHardwareInfoByType (
  IN      HARDWARE_INFO_TYPE  Type,
  IN      UINTN               TypeSize,
  IN      UINTN               TotalFileSize,
  OUT     VOID                *Data,
  OUT     UINTN               *DataSize         OPTIONAL,
  IN OUT  UINTN               *ReadIndex
  )
{
  HARDWARE_INFO_HEADER  Header;

  if ((Data == NULL) ||
      (ReadIndex == NULL) ||
      (TypeSize == 0) ||
      (Type == HardwareInfoTypeUndefined) ||
      (TotalFileSize == 0))
  {
    return EFI_INVALID_PARAMETER;
  }

  UpdateDataSize (DataSize, 0);

  while (*ReadIndex < TotalFileSize) {
    QemuFwCfgReadBytes (sizeof (Header), &Header);
    *ReadIndex += sizeof (Header);

    if ((Header.Type.Value == Type) && (Header.Size <= TypeSize)) {
      QemuFwCfgReadBytes (Header.Size, Data);

      *ReadIndex += Header.Size;
      UpdateDataSize (DataSize, Header.Size);

      return EFI_SUCCESS;
    }

    //
    // Skip the bytes corresponding to the next element as it is
    // not of the expected type and/or size. The TotalFileSize
    // and individual elements sizes should match so the size
    // check is skipped.
    //
    QemuFwCfgSkipBytes (Header.Size);
    *ReadIndex += Header.Size;
  }

  return EFI_END_OF_FILE;
}
