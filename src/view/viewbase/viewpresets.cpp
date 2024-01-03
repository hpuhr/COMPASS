
#include "viewpresets.h"

#include "view.h"
#include "files.h"
#include "compass.h"
#include "config.h"
#include "timeconv.h"

#include <set>
#include <fstream>

const std::string ViewPresets::TagName        = "name";
const std::string ViewPresets::TagCategory    = "category";
const std::string ViewPresets::TagDescription = "description";
const std::string ViewPresets::TagView        = "view";
const std::string ViewPresets::TagTimestamp   = "timestamp";
const std::string ViewPresets::TagVersion     = "app_version";
const std::string ViewPresets::TagConfig      = "view_config";

const std::string ViewPresets::DirPresets     = "view";
const std::string ViewPresets::DirPreviews    = "previews";

const std::string ViewPresets::ExtPreset      = ".json";
const std::string ViewPresets::ExtPreview     = ".png";
const std::string ViewPresets::PrefixPreset   = "preset_";

namespace
{
    std::string viewID(const View* view)
    {
        return view->classId();
    }
}

/**
*/
ViewPresets::ViewPresets() = default;

/**
*/
std::string ViewPresets::uniqueBasename(const Preset& preset) const
{
    assert(!preset.view.empty());
    assert(!preset.name.empty());

    std::string preset_dir = presetDir();
    std::string fn_base    = PrefixPreset + preset.view + "_" + Utils::Files::normalizeFilename(preset.name);

    //no preset dir yet? => return initial filename
    if (!Utils::Files::directoryExists(preset_dir))
        return fn_base;

    auto composePath = [ & ] (const std::string& basename)
    {
        return preset_dir + "/" + basename + ExtPreset;
    };

    std::string  fn_base_unique = fn_base;
    unsigned int suffix_cnt     = 1;

    //add numerical suffix until unique filename has been found
    while (Utils::Files::fileExists(composePath(fn_base_unique)))
        fn_base_unique = fn_base + "_" + std::to_string(suffix_cnt++);

    return fn_base_unique;
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
    return HOME_PRESET_DIRECTORY + "/" + DirPresets;
}

/**
*/
std::string ViewPresets::presetFilename(const Preset& preset) const
{
    assert(!preset.filename.empty());
    return preset.filename;
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
    return Utils::Files::replaceExtension(presetFilename(preset), ExtPreview);
}

/**
 * Scans the preset directory for presets and collects them.
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
 * Reads a preset json file.
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
        !data.contains(TagView)        ||
        !data.contains(TagConfig))
        return false;

    //check if view config is a valid json object
    if (data[ TagConfig ].is_null() || !data[ TagConfig ].is_object())
        return false;

    Preset p;
    p.name        = data[ TagName        ];
    p.view        = data[ TagView        ];
    p.view_config = data[ TagConfig      ];

    //optional tags
    if (data.contains(TagCategory))
        p.category = data[ TagCategory ];
    if (data.contains(TagDescription))
        p.description = data[ TagDescription ];
    if (data.contains(TagTimestamp))
        p.timestamp = data[ TagTimestamp ];
    if (data.contains(TagVersion))
        p.app_version = data[ TagVersion ];

    //key must not exist
    Key key(p.view, p.name);
    assert(presets_.count(key) == 0);

    //remember filename the preset has been read from
    p.filename = Utils::Files::getFilenameFromPath(fn);

    //try to read preview image
    std::string preview_path = previewPath(p);
    if (Utils::Files::fileExists(preview_path))
    {
        if (!p.preview.load(QString::fromStdString(preview_path)))
            logwrn << "ViewPresets: readPreset: could not read preview image for preset '" << p.name << "'";
    }

    //store preset
    presets_[ key ] = p;

    return true;
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
    assert(!preset.filename.empty());

    nlohmann::json obj;
    obj[ TagView        ] = preset.view;
    obj[ TagName        ] = preset.name;
    obj[ TagDescription ] = preset.description;
    obj[ TagCategory    ] = preset.category;
    obj[ TagConfig      ] = preset.view_config;

    if (!preset.timestamp.empty())
        obj[ TagTimestamp   ] = preset.timestamp;
    if (!preset.app_version.empty())
        obj[ TagVersion ] = preset.app_version;

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

    //@TODO: before writing make a backup copy of the file we could revert to if the write fails
    std::ofstream fileout(preset_path, std::ios::out);
    if (!fileout.is_open())
        return false;

    fileout << content;
    if (!fileout)
        return false;

    return true;
}

/**
 * Creates a new preset for the given view and optionally a preview image.
 */
bool ViewPresets::createPreset(const View* view, 
                               const std::string& name,
                               const std::string& category,
                               const std::string& description,
                               bool create_preview)
{
    assert(view);
    assert(!name.empty());

    //config preset
    Preset p;
    p.name        = name;
    p.view        = viewID(view);
    p.description = description;
    p.category    = category;

    //update view config + preview
    updatePresetConfig(p, view, create_preview);

    //create configured preset
    return createPreset(p, nullptr);
}

/**
 * Creates the given preset on disk and adds it to the collection of presets.
 * The view can be passed to set the preset's view id (otherwise it needs to be set beforehand).
 */
bool ViewPresets::createPreset(const Preset& preset, const View* view)
{
    return createPreset(preset, view, true);
}

/**
 * Creates the given preset on disk and adds it to the collection of presets.
 * The view can be passed to set the preset's view id (otherwise it needs to be set beforehand).
 * Internal version.
 */
bool ViewPresets::createPreset(const Preset& preset, const View* view, bool signal_changes)
{
    assert(!preset.name.empty());

    Preset p = preset;

    //update view info?
    if (view)
        p.view = viewID(view);

    assert(!p.view.empty());

    auto key = p.key();
    assert(presets_.count(key) == 0);

    //create unique filename
    p.filename = uniqueBasename(p) + ExtPreset;

    //try to write
    if (!writePreset(p))
    {
        logwrn << "ViewPresets: createPreset: could not write view preset '" << p.name << "'";
        return false;
    }

    //add to presets only if write succeeds
    presets_[ key ] = p;

    if (signal_changes)
        emit presetAdded(key);

    return true;
}


/**
 * Removes the preset of the given key.
 */
void ViewPresets::removePreset(const Key& key)
{
    removePreset(key, true);
}

/**
 * Removes the preset of the given key.
 * Internal version.
 */
void ViewPresets::removePreset(const Key& key, bool signal_changes)
{
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

    if (signal_changes)
        emit presetRemoved(key);
}

/**
 * Renames the preset of the given key to the given name.
 * Will cause the original file to be deleted and a file to be generated under the new name.
 */
bool ViewPresets::renamePreset(const Key& key, const std::string& new_name)
{
    return renamePreset(key, new_name, true);
}

/**
 * Renames the preset of the given key to the given name.
 * Will cause the original file to be deleted and a file to be generated under the new name.
 * Internal version.
 */
bool ViewPresets::renamePreset(const Key& key, const std::string& new_name, bool signal_changes)
{
    assert(presets_.count(key) > 0);
    assert(!new_name.empty() && new_name != key.second);

    //copy preset and set new name
    Preset preset = presets_.at(key);
    preset.name = new_name;

    //create preset under new name
    if (!createPreset(preset, nullptr, false))
        return false;

    //only if creation succeeds remove preset under old name
    removePreset(key, false);

    if (signal_changes)
        emit presetRenamed(key, preset.key());

    return true;
}

/**
 * Updates the preset under the given key to the passed preset.
 * Might cause a rename. If the update fails the old preset version will be restored.
 */
bool ViewPresets::updatePreset(const Key& key, 
                               const Preset* preset, 
                               const View* view, 
                               UpdateMode mode,
                               bool update_preview)
{
    return updatePreset(key, preset, view, mode, update_preview, true);
}

/**
 * Updates the preset under the given key to the data of the passed preset.
 * If a view is given the presets config will be updated to the view config.
 * Internal version.
 */
bool ViewPresets::updatePreset(const Key& key, 
                               const Preset* preset,
                               const View* view, 
                               UpdateMode mode,
                               bool update_preview, 
                               bool signal_changes)
{
    assert(presets_.count(key) > 0);
    assert(preset != nullptr || view != nullptr);

    auto& preset_cur = presets_.at(key);

    //backup current preset version
    auto preset_backup = preset_cur;

    //copy description etc from preset
    if (mode == UpdateMode::MetaData || mode == UpdateMode::All)
    {
        if (preset)
        {
            preset_cur.description = preset->description;
            preset_cur.category    = preset->category;
        }
    }

    //change config?
    if (mode == UpdateMode::ViewConfig || mode == UpdateMode::All)
    {
        if (view)
        {
            assert(viewID(view) == preset_cur.view);

            //view provided => update to view config
            updatePresetConfig(preset_cur, view, update_preview);
        }
        else if (preset && !preset->view_config.is_null())
        {
            assert(preset_cur.view == preset->view);

            //config provided in passed preset => copy config related data
            preset_cur.view_config = preset->view_config;
            preset_cur.preview     = preset->preview;
            preset_cur.timestamp   = preset->timestamp;
            preset_cur.app_version = preset->app_version;
        }
    }

    //try to write
    if (!writePreset(preset_cur))
    {
        //fail => revert to old version
        preset_cur = preset_backup;
        return false;
    }

    if (signal_changes)
        emit presetUpdated(key);

    return true;
}

/**
 * Writes the preset of the given key to disk.
 * Internal version.
 */
bool ViewPresets::writePreset(const Key& key) const
{
    assert(presets_.count(key) != 0);

    const Preset& p = presets_.at(key);

    if (!writePreset(p))
    {
        logwrn << "ViewPresets: writePreset: could not write view preset '" << key.second << "'";
        return false;
    }

    return true;
}

/**
 * Updates the presets "stamp", meaning timestamp and app version.
 * This stamp is usually updated when setting a new view config.
*/
void ViewPresets::updatePresetStamp(Preset& preset)
{
    //add timestamp
    preset.timestamp = Utils::Time::toString(Utils::Time::currentUTCTime());

    //add app version
    preset.app_version = COMPASS::instance().config().getString("version");
}

/**
 * Generates a preview image of the given view.
 */
QImage ViewPresets::renderPreview(const View* view)
{
    assert(view);
    
    auto preview = view->renderData();
    return preview.scaled(PreviewMaxSize, PreviewMaxSize, Qt::AspectRatioMode::KeepAspectRatio);
}

/**
 * Updates the presets config to the passed view's config, and optionally creates a preview image.
 * Preferably use this method to update the presets view config, since it also updates the preset's stamp
 * on config change.
 */
void ViewPresets::updatePresetConfig(Preset& preset, const View* view, bool update_preview)
{
    assert(view);
    assert(preset.view.empty() || viewID(view) == preset.view);

    //collect json config
    view->generateJSON(preset.view_config, Configurable::JSONExportType::Preset);

    if (update_preview)
        preset.preview = renderPreview(view);

    //update signature on config modify
    updatePresetStamp(preset);
}

/**
*/
bool ViewPresets::keyIsView(const Key& key, const View* view)
{
    assert(view);
    return (viewID(view) == key.first);
}

/**
*/
const ViewPresets::Presets& ViewPresets::presets() const
{
    return presets_;
}

/**
*/
ViewPresets::Presets& ViewPresets::presets()
{
    return presets_;
}

/**
 * Returns the preset keys for presets belonging to the given view.
 */
std::vector<ViewPresets::Key> ViewPresets::keysFor(const View* view) const
{
    assert(view);

    std::vector<Key> keys;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view))
            keys.push_back(p.first);

    return keys;
}

/**
 * Returns the preset keys for presets belonging to the given category.
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
 * Returns the preset keys for presets belonging to the given view and category.
 */
std::vector<ViewPresets::Key> ViewPresets::keysFor(const View* view, 
                                                   const std::string& category) const
{
    assert(view);

    std::vector<Key> keys;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view) && p.second.category == category)
            keys.push_back(p.first);

    return keys;
}

/**
 * Returns all existing categories.
 */
std::vector<std::string> ViewPresets::categories() const
{
    std::set<std::string> categories;

    for (const auto& p : presets_)
        categories.insert(p.second.category);

    return std::vector<std::string>(categories.begin(), categories.end());
}

/**
 * Returns all categories existing for the given view.
 */
std::vector<std::string> ViewPresets::categories(const View* view) const
{
    assert(view);

    std::set<std::string> categories;

    for (const auto& p : presets_)
        if (p.second.view == viewID(view))
            categories.insert(p.second.category);

    return std::vector<std::string>(categories.begin(), categories.end());
}

/**
*/
bool ViewPresets::hasPreset(const View* view, const std::string& name) const
{
    assert(view);

    return hasPreset(Key(viewID(view), name));
}

/**
*/
bool ViewPresets::hasPreset(const Key& key) const
{
    return presets_.count(key) > 0;
}
