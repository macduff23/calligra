/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CALLIGRA_SHEETS_CELL_TOOL_FACTORY
#define CALLIGRA_SHEETS_CELL_TOOL_FACTORY

#include <KoToolFactoryBase.h>

#include "sheets_common_export.h"

namespace Calligra
{
namespace Sheets
{

/**
 * The Factory, that creates a CellTool.
 */
class CALLIGRA_SHEETS_COMMON_EXPORT CellToolFactory : public KoToolFactoryBase
{
public:
    explicit CellToolFactory(const QString& id);
    ~CellToolFactory();

    KoToolBase* createTool(KoCanvasBase* canvas);

    void setPriority(int priority);
    void setToolTip(const QString& toolTip);
    void setIconName(const char *iconName);
    void setIconName(const QString &iconName);
};

} // namespace Sheets
} // namespace Calligra

#endif // CALLIGRA_SHEETS_CELL_TOOL_FACTORY
