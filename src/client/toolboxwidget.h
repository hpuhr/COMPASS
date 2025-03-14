/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

#include <QWidget>
#include <QIcon>

/**
 */
class ToolBoxWidget : public QWidget
{
public:
    ToolBoxWidget(QWidget* parent = nullptr);
    virtual ~ToolBoxWidget();

    virtual QIcon toolIcon() const = 0;
    virtual std::string toolName() const = 0;
    virtual std::string toolInfo() const = 0;
};

/**
 */
class WrappedToolBoxWidget : public ToolBoxWidget
{
public:
    WrappedToolBoxWidget(QWidget* w, 
                         const std::string& name,
                         const std::string& info,
                         const QIcon& icon,
                         QWidget* parent = nullptr);
    virtual ~WrappedToolBoxWidget();

    QIcon toolIcon() const override final;
    std::string toolName() const override final;
    std::string toolInfo() const override final;

private:
    std::string name_;
    std::string info_;
    QIcon       icon_;
};
