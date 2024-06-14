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

#include <string>
#include <vector>

#include <QColor>

/**
 * A raw histogram bin.
 */
struct RawHistogramBin
{
    enum class Tag
    {
        Standard = 0,
        NullValues,
        OutOfRange,
        CouldNotInsert
    };

    RawHistogramBin();
    RawHistogramBin(uint32_t cnt, 
                    const std::string& lbl, 
                    Tag t = Tag::Standard,
                    const std::string& lbl_min = "",
                    const std::string& lbl_max = "");

    bool isStandardBin() const;

    Tag         tag   = Tag::Standard; // bin tag
    uint32_t    count = 0;             // bin count

    std::string label;                 // label for bin
    std::string label_min;             // label for bin range min
    std::string label_max;             // label for bin range max
};

/**
 * A raw histogram.
 */
class RawHistogram
{
public:
    typedef std::vector<RawHistogramBin> RawHistogramBins;

    RawHistogram();
    virtual ~RawHistogram();

    void clear();

    void addBin(const RawHistogramBin& bin);
    void addBins(const RawHistogramBins& bins);
    void setBins(const RawHistogramBins& bins);
    size_t numBins() const;
    const RawHistogramBins& getBins() const;

    bool fromJSON(const nlohmann::json& data);
    nlohmann::json toJSON() const;

    static const std::string TagBins;
    static const std::string TagBinTag;
    static const std::string TagBinCount;
    static const std::string TagBinLabel;
    static const std::string TagBinLabelMin;
    static const std::string TagBinLabelMax;

private:
    RawHistogramBins bins_;
};

/**
 * Collection of raw histograms of the same resolution.
 */
class RawHistogramCollection
{
public:
    struct Layer
    {
        RawHistogram histogram;
        std::string  name;
        QColor       color;
    };

    RawHistogramCollection();
    virtual ~RawHistogramCollection();

    bool hasData() const;
    void clear();

    bool addLayer(const RawHistogram& histogram,
                  const std::string& name = "",
                  const QColor& color = Qt::black);
    bool addLayer(const Layer& layer);

    const std::vector<Layer>& layers() const;

    size_t numBins() const;
    size_t numLayers() const;

    std::vector<std::string> labels() const;

    bool fromJSON(const nlohmann::json& data);
    nlohmann::json toJSON() const;

    static const std::string TagLayers;
    static const std::string TagHistogram;
    static const std::string TagName;
    static const std::string TagColor;

private:
    std::vector<Layer> layers_;
};
