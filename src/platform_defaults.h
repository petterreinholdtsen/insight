#ifndef PLATFORM_DEFAULTS_H
#define PLATFORM_DEFAULTS_H

/*****************************************************************************
**
**  Definition of the platform defaults, to have them all in one location.
**
**  Creation date:  2019-11-07
**  Created by:     Petter Reinholdtsen
**
**  Copyright (c) 2019 Petter Reinholdtsen.
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License as
**  published by the Free Software Foundation; either version 3 of the
**  License, or any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include <QString>

#if defined(_WIN32)
static QString INDEXER_TOOL =
  "index.cmd %REPORTS_DIR% %ATTACHMENT_DIR% %NAME%";
static QString EMAIL_APPLICATION =
  "c:\\Program Files (x86)\\Microsoft Office\\root\\Office16\\OUTLOOK.EXE";
static EMAIL_ARGUMENTS =
  "/c ipm.note /m 'mailto:johndoe@domain.com&subject=Report' /a %ATTACHMENT_FILENAME%";
#elif defined(unix)
// FIXME this part is not tested
static QString INDEXER_TOOL =
  "index %REPORTS_DIR% %ATTACHMENT_DIR% %NAME%";
static QString EMAIL_APPLICATION =
  "xterm -e mutt";
static QString EMAIL_ARGUMENTS =
  "-a %ATTACHMENT_FILENAME% -s Report johndoe@domain.com ";
#else
#  error "Unsupported platform!"
#endif

#endif // PLATFORM_DEFAULTS_H
