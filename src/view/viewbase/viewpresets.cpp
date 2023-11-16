
#include "viewpresets.h"

#include "view.h"
#include "files.h"
#include "compass.h"

#include <set>
#include <fstream>

namespace
{
    std::string viewID(View* view)
    {
        return view->classId();
    }
}

const std::string ViewPresets::TagName        = "name";
const std::string ViewPresets::TagCategory    = "category";
const std::string ViewPresets::TagDescription = "description";
const std::string ViewPresets::TagView        = "view";
const std::string ViewPresets::TagTimestamp   = "timestamp";
const std::string ViewPresets::TagConfig      = "view_config";

const std::string ViewPresets::DirPresets     = "view_presets";
const std::string ViewPresets::DirPreviews    = "previews";

const std::string ViewPresets::ExtPreset      = ".json";
const std::string ViewPresets::ExtPreview     = ".png";
const std::string ViewPresets::PrefixPreset   = "preset_";

/**
*/
ViewPresets::ViewPresets() = default;

/**
*/
bool ViewPresets::scanForPresets()
{
    presets_.clear();

    auto preset_dir  = presetDir();

    if (!Utils::Files::directoryExists(preset_dir))
    {
        logwrn << "ViewPresets: scanForPresets: view preset directory not found";
        return true;
    }

    auto files = Utils::Files::getFilesInDirectory(preset_dir);

    for (const auto& f : files)
    {
        if (f.startsWith(QString::fromStdString(PrefixPreset)) && 
            f.endsWith(QString::fromStdString(ExtPreset)) &&
            !readPreset(preset_dir + "/" + f.toStdString()))
            logwrn << "ViewPresets: scanForPresets: could not read view preset from " << f.toStdString();
    }

    loginf << "ViewPresets: scanForPresets: read " << presets_.size() << " preset(s)";

    return true;
}

/**
*/
bool ViewPresets::readPreset(const std::string& fn)
{
    if (!Utils::Files::fileExists(fn))
        return false;

    //read file to json object
    std::ifstream f(fn);
    if (!f.is_open())
        return false;

    nlohmann::json data = nlohmann::json::parse(f);
    if (data.is_null() || !data.is_object())
        return false;

    //everything there?
    if (!data.contains(TagName)        ||
        !data.contains(TagCategory)    ||
        !data.contains(TagDescription) ||
        !data.contains(TagView)        ||
        !data.contains(TagTimestamp)   ||
        !data.contains(TagConfig))
        return false;

    //check if view config is a valid json object
    if (data[ TagConfig ].is_null() || !data[ TagConfig ].is_object())
        return false;

    Preset p;
    p.category    = data[ TagCategory    ];
    p.name        = data[ TagName        ];
    p.description = data[ TagDescription ];
    p.view        = data[ TagView        ];
    p.timestamp   = data[ TagTimestamp   ];
    p.view_config = data[ TagConfig      ];

    //key must not exist
    Key key(p.view, p.name);
    assert(presets_.count(key) == 0);

    //try to read preview image
    std::string preview_path = previewPath(p);
    if (Utils::Files::fileExists(preview_path))
    {
        if (!p.preview.load(QString::fromStdString(preview_path)))
            logwrn << "ViewPresets: readPreset: could not read preview image for preset '" << p.name << "'";
    }

    presets_[ key ] = p;

    return true;
}

/**
*/
QImage ViewPresets::renderPreview(View* view) const
{
    auto preview = view->renderData();
    return preview.scaled(PreviewMaxSize, PreviewMaxSize, Qt::AspectRatioMode::KeepAspectRatio);
}

/**
*/
bool ViewPresets::createPreset(View* view, 
                               const std::string& name,
                               const std::string& category,
                               const std::string& description,
                               bool create_preview)
{
    assert(view);
    assert(!name.empty());

    Key key(viewID(view), name);
    assert(presets_.count(key) == 0);

    Preset& p = presets_[ key ];

    p.view        = viewID(view);
    p.name        = name;
    p.description = description;
    p.category    = category;

    //@TODO: create timestamp
    p.timestamp = "";

    //collect json config
    view->generateJSON(p.view_config, Configurable::JSONExportType::Preset);

    //auto-create create_preview?
    if (create_preview)
        p.preview = renderPreview(view);

    bool ok = writePreset(p);
    if (!ok)
        logwrn << "ViewPresets: createPreset: could not write view preset '" << name << "'";

    return ok;
}

/**
*/
bool ViewPresets::updatePreset(View* view, 
                               const std::string& name,
                               const std::string& category,
                               const std::string& description,
                               bool update_view_config,
                               bool create_preview)
{
    assert(view);
    assert(!name.empty());

    Key key(viewID(view), name);
    assert(presets_.count(key) != 0);

    Preset& p = presets_[ key ];
    p.description = description;
    p.category    = category;

    //@TODO: update timestamp
    p.timestamp = "";

    //collect json config?
    if (update_view_config)
        view->generateJSON(p.view_config, Configurable::JSONExportType::Preset);

    //auto-create preview?
    if (create_preview)
        p.preview = renderPreview(view);

    bool ok = writePreset(p);
    if (!ok)
        logwrn << "ViewPresets: updatePreset: could not write view preset '" << name << "'";

    return ok;
}

/**
*/
std::string ViewPresets::presetBaseName(const Preset& preset) const
{
    return PrefixPreset + preset.view + "_" + Utils::Files::normalizeFilename(preset.name);
}

/**
*/
std::string ViewPresets::presetPath(const Preset& preset) const
{
    return presetDir() + "/" + presetFilename(preset);
}

/**
*/
std::string ViewPresets::presetDir() const
{
    return HOME_DATA_DIRECTORY + "/" + DirPresets;
}

/**
*/
std::string ViewPresets::presetFilename(const Preset& preset) const
{
    return presetBaseName(preset) + ExtPreset;
}

/**
*/
std::string ViewPresets::previewPath(const Preset& preset) const
{
    return previewDir() + "/" + previewFilename(preset);
}

/**
*/
std::string ViewPresets::previewDir() const
{
    return presetDir() + "/" + DirPreviews;
}

/**
*/
std::string ViewPresets::previewFilename(const Preset& preset) const
{
    return presetBaseName(preset) + ExtPreview;
}

/**
*/
bool ViewPresets::writePreview(const Preset& preset) const
{
    //nothing to do?
    if (preset.preview.isNull())
        return true;

    //obtain unique preview path
    std::string preview_path = previewPath(preset);
    std::string preview_dir  = previewDir();

    //create directories if needed
    if (!Utils::Files::directoryExists(preview_dir) && 
        !Utils::Files::createMissingDirectories(preview_dir))
        return false;

    //write preview
    if (!preset.preview.save(QString::fromStdString(preview_path)))
        return false;

    return true;
}

/**
*/
bool ViewPresets::writePreset(const Preset& preset) const
{
    nlohmann::json obj;
    obj[ TagView        ] = preset.view;
    obj[ TagName        ] = preset.name;
    obj[ TagDescription ] = preset.description;
    obj[ TagCategory    ] = preset.category;
    obj[ TagTimestamp   ] = preset.timestamp;
    obj[ TagConfig      ] = preset.view_config;

    //try to write preview image
    if (!preset.preview.isNull() && !writePreview(preset))
        logwrn << "ViewPresets: writePreset: preview image for view preset '" << preset.name << "' could not be written";
    
    //obtain unique preset path
    std::string preset_path = presetPath(preset);
    std::string preset_dir  = presetDir();

    //create directories if needed
    if (!Utils::Files::directoryExists(preset_dir) &&
        !Utils::Files::createMissingDirectories(preset_dir))
        return false;

    //write json content
    std::string content = obj.dump(4);
    if (content.empty())
        return false;

    std::ofstream fileout(preset_path, std::ios::out);
    if (!fileout.is_open())
        return false;

    fileout << content;
    if (!fileout)
        return false;

    return true;
}

/**
*/
void ViewPresets::removePreset(View* view, 
                               const std::string& name)
{
    Key key(viewID(view), name);
    assert(presets_.count(key) > 0);

    const auto& preset = presets_.at(key);

    std::string preset_path  = presetPath(preset);
    std::string preview_path = previewPath(preset);

    assert(Utils::Files::fileExists(preset_path));

    //delete preview if exists
    if (Utils::Files::fileExists(preview_path))
        Utils::Files::deleteFile(preview_path);

    //delete preset
    Utils::Files::deleteFile(preset_path);

    //remove preset
    presets_.erase(key);
}

/**
*/
const ViewPresets::Presets& ViewPresets::presets() const
{
    return presets_;
}

/**
*/
std::vector<ViewPresets::Key> ViewPresets::keysFor(View* view) const
{
    std::vector<Key> keys;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view))
            keys.push_back(p.first);

    return keys;
}

/**
*/
std::vector<ViewPresets::Key> ViewPresets::keysFor(const std::string& category) const
{
    std::vector<Key> keys;

    for (const auto& p : presets_)
        if (p.second.category == category)
            keys.push_back(p.first);

    return keys;
}

/**
*/
std::vector<ViewPresets::Key> ViewPresets::keysFor(View* view, 
                                                   const std::string& category) const
{
    std::vector<Key> keys;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view) && p.second.category == category)
            keys.push_back(p.first);

    return keys;
}

/**
*/
std::vector<std::string> ViewPresets::categories() const
{
    std::set<std::string> categories;

    for (const auto& p : presets_)
        categories.insert(p.second.category);

    return std::vector<std::string>(categories.begin(), categories.end());
}

/**
*/
std::vector<std::string> ViewPresets::categories(View* view) const
{
    std::set<std::string> categories;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view))
            categories.insert(p.second.category);

    return std::vector<std::string>(categories.begin(), categories.end());
}

/**
*/
bool ViewPresets::hasPreset(View* view, const std::string& name) const
{
    Key key(viewID(view), name);
    return presets_.count(key) > 0;
}
