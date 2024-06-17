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

#include "histogram_raw.h"

const std::string RawHistogram::TagBins        = "bins";
const std::string RawHistogram::TagBinTag      = "tag";
const std::string RawHistogram::TagBinCount    = "count";
const std::string RawHistogram::TagBinLabel    = "label";
const std::string RawHistogram::TagBinLabelMin = "label_min";
const std::string RawHistogram::TagBinLabelMax = "label_max";

/*******************************************************************************************
 * RawHistogramBin
 *******************************************************************************************/

/**
*/
RawHistogramBin::RawHistogramBin() {}

/**
*/
RawHistogramBin::RawHistogramBin(uint32_t cnt, 
                                 const std::string& lbl, 
                                 Tag t,
                                 const std::string& lbl_min,
                                 const std::string& lbl_max)
:   tag      (t      )
,   count    (cnt    )
,   label    (lbl    )
,   label_min(lbl_min)
,   label_max(lbl_max)
{
}

/**
*/
bool RawHistogramBin::isStandardBin() const 
{
    return (tag == Tag::Standard);
}

/*******************************************************************************************
 * RawHistogram
 *******************************************************************************************/

/**
*/
RawHistogram::RawHistogram() = default;

/**
*/
RawHistogram::~RawHistogram() = default;

/**
*/
void RawHistogram::clear()
{
    bins_.clear();
}

/**
*/
void RawHistogram::addBin(const RawHistogramBin& bin)
{
    bins_.push_back(bin);
}

/**
*/
void RawHistogram::addBins(const RawHistogramBins& bins)
{
    bins_.insert(bins_.end(), bins.begin(), bins.end());
}

/**
*/
void RawHistogram::setBins(const RawHistogramBins& bins)
{
    bins_ = bins;
}

/**
*/
size_t RawHistogram::numBins() const
{
    return bins_.size();
}

/**
*/
const RawHistogram::RawHistogramBins& RawHistogram::getBins() const
{
    return bins_;
}

/**
*/
bool RawHistogram::fromJSON(const nlohmann::json& data)
{
    clear();

    if (!data.is_object() || !data.contains(TagBins))
        return false;

    try
    {
        const auto& jbins = data[ TagBins ];
        if (!jbins.is_array())
            return false;

        for (const auto& jbin : jbins)
        {
            if (!jbin.is_object() ||
                !jbin.contains(TagBinCount) ||
                !jbin.contains(TagBinLabel))
                return false;

            RawHistogramBin b;
            b.count = jbin[ TagBinCount ];
            b.label = jbin[ TagBinLabel ];

            if (jbin.contains(TagBinTag))
            {
                int t = jbin[ TagBinTag ];
                b.tag = static_cast<RawHistogramBin::Tag>(t);
            }

            if (jbin.contains(TagBinLabelMin))
                b.label_min = jbin[ TagBinLabelMin ];

            if (jbin.contains(TagBinLabelMax))
                b.label_max = jbin[ TagBinLabelMax ];

            bins_.push_back(b);
        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/**
*/
nlohmann::json RawHistogram::toJSON() const
{
    nlohmann::json j = nlohmann::json::object();

    nlohmann::json jbins = nlohmann::json::array();

    for (const auto& b : bins_)
    {
        nlohmann::json jbin = nlohmann::json::object();

        jbin[ TagBinTag      ] = (int)b.tag;
        jbin[ TagBinCount    ] = b.count;
        jbin[ TagBinLabel    ] = b.label;
        jbin[ TagBinLabelMin ] = b.label_min;
        jbin[ TagBinLabelMax ] = b.label_max;

        jbins.push_back(jbin);
    }

    j[ TagBins ] = jbins;

    return j;
}

/*******************************************************************************************
 * RawHistogramCollection
 *******************************************************************************************/

const std::string RawHistogramCollection::TagLayers    = "layers";
const std::string RawHistogramCollection::TagHistogram = "histogram";
const std::string RawHistogramCollection::TagName      = "name";
const std::string RawHistogramCollection::TagColor     = "color";

/**
*/
RawHistogramCollection::RawHistogramCollection() = default;

/**
*/
RawHistogramCollection::~RawHistogramCollection() = default;

/**
*/
bool RawHistogramCollection::hasData() const
{
    return (numLayers() > 0 && layers_.begin()->histogram.numBins() > 0);
}

/**
*/
void RawHistogramCollection::clear()
{
    layers_.clear();
}

/**
*/
bool RawHistogramCollection::addLayer(const RawHistogram& histogram,
                                      const std::string& name,
                                      const QColor& color)
{
    Layer l;
    l.histogram = histogram;
    l.name      = name;
    l.color     = color;

    return addLayer(l);
}

/**
*/
bool RawHistogramCollection::addLayer(const Layer& layer)
{
    bool bin_count_ok = (layers_.empty() || layers_.rbegin()->histogram.numBins() == layer.histogram.numBins());

    assert(bin_count_ok);

    if (bin_count_ok)
        layers_.push_back(layer);

    return bin_count_ok;
}

/**
*/
const std::vector<RawHistogramCollection::Layer>& RawHistogramCollection::layers() const
{
    return layers_;
}

/**
*/
size_t RawHistogramCollection::numLayers() const
{
    return layers_.size();
}

/**
*/
size_t RawHistogramCollection::numBins() const
{
    if (!hasData())
        return 0;

    return (layers_.begin()->histogram.numBins());
}

/**
*/
std::vector<std::string> RawHistogramCollection::labels() const
{
    if (!hasData())
        return {};

    size_t nbins = numBins();

    const auto& bins = layers_.begin()->histogram.getBins();

    std::vector<std::string> l(nbins);

    for (size_t i = 0; i < nbins; ++i)
        l[ i ] = bins[ i ].label;

    return l;
}

/**
*/
bool RawHistogramCollection::fromJSON(const nlohmann::json& data)
{
    clear();

    if (!data.is_object() || !data.contains(TagLayers))
        return false;

    try
    {
        const auto& jlayers = data[ TagLayers ];
        if (!jlayers.is_array())
            return false;

        for (const auto& jlayer : jlayers)
        {
            if (!jlayer.is_object() || !jlayer.contains(TagHistogram))
                return false;

            Layer l;

            if (!l.histogram.fromJSON(jlayer[ TagHistogram ]))
                return false;

            if (jlayer.contains(TagName))
                l.name = jlayer[ TagName ];

            if (jlayer.contains(TagColor))
            {
                std::string cname = jlayer[ TagColor ];
                l.color = QColor(QString::fromStdString(cname));
            }

            if (!addLayer(l))
                return false;
        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/**
*/
nlohmann::json RawHistogramCollection::toJSON() const
{
    nlohmann::json j;

    nlohmann::json jlayers = nlohmann::json::array();

    for (const auto& l : layers_)
    {
        nlohmann::json jlayer = nlohmann::json::object();

        jlayer[ TagHistogram ] = l.histogram.toJSON();
        jlayer[ TagName      ] = l.name;
        jlayer[ TagColor     ] = l.color.name().toStdString();

        jlayers.push_back(jlayer);
    }

    j[ TagLayers ] = jlayers;

    return j;
}
