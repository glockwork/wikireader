/*
 * Wiki Handling Tool
 *
 * Copyright (C) 2008 Openmoko Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CreateIndex_h
#define CreateIndex_h

#include "ArticleHandler.h" 
#include <QFile>

/**
 * Extract Titles and build a simple index
 *   Title => Hash
 */
class CreateIndex : public ArticleHandler {
public:
    CreateIndex(const QString& outputFile);

    void parsingStarts();
    void parsingFinished();
    void handleArticle(const Article&);

private:
    QString m_filePath;
    QFile m_file;
};

#endif
