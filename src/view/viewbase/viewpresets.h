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
class ViewPresets : public QObject
{
    Q_OBJECT
public:
    typedef std::pair<std::string, std::string> Key;

    /**
     * Struct defining a view preset.
     */
    struct Preset
    {
        //returns a unique key, being a combination of the view's class id and the preset name
        Key key() const { return Key(view, name); }

        //serialized
        std::string    name;          // unique name of the preset
        std::string    category;      // category of the preset
        std::string    description;   // description of what the preset is about
        std::string    view;          // view the preset belongs to (= the view's configurable class id)
        std::string    timestamp;     // timestamp the preset config was generated
        std::string    app_version;   // version of the application the preset has been generated with
        nlohmann::json view_config;   // view configuration as json blob

        //not serialized
        std::string    filename;      // filename of the preset on disk
        QImage         preview;       // a preview image generated from the view using the stored configuration
    };

    typedef std::map<Key, Preset> Presets;

    ViewPresets();
    virtual ~ViewPresets() = default;

    bool scanForPresets();

    bool createPreset(View* view,
                      const std::string& name,
                      const std::string& category,
                      const std::string& description,
                      bool create_preview = true);
    bool createPreset(const Preset& preset, const View* view);
    void removePreset(const Key& key);
    bool renamePreset(const Key& key, const std::string& new_name);
    bool updatePreset(const Key& key, const Preset& preset);
    
    bool hasPreset(const View* view, const std::string& name) const;
    bool hasPreset(const Key& key) const;
    bool nameExists(const std::string& name, const View* view) const;

    static void updatePresetConfig(Preset& preset, View* view, bool update_preview = true);
    static QImage renderPreview(View* view);

    static bool keyIsView(const Key& key, View* view);

    const Presets& presets() const;
    Presets& presets();

    std::vector<Key> keysFor(const View* view) const;
    std::vector<Key> keysFor(const std::string& category) const;
    std::vector<Key> keysFor(const View* view, 
                             const std::string& category) const;

    std::vector<std::string> categories() const;
    std::vector<std::string> categories(const View* view) const;

    static const std::string TagName;
    static const std::string TagCategory;
    static const std::string TagDescription;
    static const std::string TagView;
    static const std::string TagTimestamp;
    static const std::string TagVersion;
    static const std::string TagConfig;

    static const std::string DirPresets;
    static const std::string DirPreviews;

    static const std::string ExtPreset;
    static const std::string ExtPreview;
    static const std::string PrefixPreset;

    static const int PreviewMaxSize = 100;

signals:
    void presetUpdated(Key key_before, Key key_after);
    void presetAdded(Key key);
    void presetRemoved(Key key);
    void presetRenamed(Key key_old, Key key_new);

private:
    bool writePreview(const Preset& preset) const;
    bool writePreset(const Preset& preset) const;
    bool writePreset(const Key& key) const;
    bool readPreset(const std::string& fn);

    //internal versions
    bool createPreset(const Preset& preset, const View* view, bool signal_changes);
    void removePreset(const Key& key, bool signal_changes);
    bool renamePreset(const Key& key, const std::string& new_name, bool signal_changes);
    bool updatePreset(const Key& key, const Preset& preset, bool signal_changes);

    std::string uniqueBasename(const Preset& preset) const;

    std::string presetDir() const;
    std::string presetFilename(const Preset& preset) const;
    std::string presetPath(const Preset& preset) const;

    std::string previewDir() const;
    std::string previewFilename(const Preset& preset) const;
    std::string previewPath(const Preset& preset) const;
    
    Presets presets_;
};
