// SPDX-FileCopyrightText: 2023 The libsensible Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifdef _WIN32
# include "sensible-timing-windows.c"
#else
# include "sensible-timing-posix.c"
#endif
