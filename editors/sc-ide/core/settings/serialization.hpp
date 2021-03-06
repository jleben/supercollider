/*
    SuperCollider Qt IDE
    Copyright (c) 2012 Jakob Leben & Tim Blechmann
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef SCIDE_CORE_SETTINGS_SERIALIZATION_HPP_INCLUDED
#define SCIDE_CORE_SETTINGS_SERIALIZATION_HPP_INCLUDED

#include <QSettings>
#include <QTextCharFormat>

Q_DECLARE_METATYPE( QVector<QVariant> );
Q_DECLARE_METATYPE( QTextCharFormat );

namespace ScIDE { namespace Settings {

QSettings::Format serializationFormat();


}} // namespace ScIDE::Settings

#endif // SCIDE_CORE_SETTINGS_SERIALIZATION_HPP_INCLUDED
