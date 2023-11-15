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

#include "json.h"

#include <map>
#include <string>

#include <QImage>

class View;

/**
 * Collection of presets for all kinds of views.
 */
class ViewPresets
{
public:
    struct Preset
    {
        std::string    name;
        std::string    category;
        std::string    description;
        std::string    view;
        std::string    timestamp;
        QImage         preview;
        nlohmann::json view_config;
    };

    typedef std::pair<std::string, std::string> Key;
    typedef std::map<Key, Preset>               Presets;

    ViewPresets();
    virtual ~ViewPresets() = default;

    bool scanForPresets();
    bool createPreset(View* view, 
                      const std::string& name,
                      const std::string& category,
                      const std::string& description,
                      bool create_preview = false);
    void removePreset(View* view, 
                      const std::string& name);
    bool updatePreset(View* view, 
                      const std::string& name,
                      const std::string& category,
                      const std::string& description,
                      bool create_preview = false);

    bool hasPreset(View* view, const std::string& name) const;

    const Presets& presets() const;
    std::vector<Key> keysFor(View* view) const;
    std::vector<Key> keysFor(const std::string& category) const;
    std::vector<Key> keysFor(View* view, 
                             const std::string& category) const;

    std::vector<std::string> categories() const;
    std::vector<std::string> categories(View* view) const;

    static const std::string TagName;
    static const std::string TagCategory;
    static const std::string TagDescription;
    static const std::string TagView;
    static const std::string TagTimestamp;
    static const std::string TagConfig;

    static const std::string DirPresets;
    static const std::string DirPreviews;

    static const std::string ExtPreset;
    static const std::string ExtPreview;
    static const std::string PrefixPreset;

    static const int PreviewMaxSize = 100;

private:
    bool writePreview(const Preset& preset) const;
    bool writePreset(const Preset& preset) const;
    bool readPreset(const std::string& fn);

    QImage renderPreview(View* view) const;

    std::string presetBaseName(const Preset& preset) const;

    std::string presetDir() const;
    std::string presetFilename(const Preset& preset) const;
    std::string presetPath(const Preset& preset) const;

    std::string previewDir() const;
    std::string previewFilename(const Preset& preset) const;
    std::string previewPath(const Preset& preset) const;
    
    Presets presets_;
};
