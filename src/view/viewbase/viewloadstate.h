
#pragma once

#include <boost/optional.hpp>

#include <QString>
#include <QColor>

namespace view
{
    /**
    */
    struct LoadState
    {
        enum class State
        {
            NoData = 0,
            Loading,
            Loaded,
            ReloadRequired
        };

        LoadState() {}
        LoadState(State s, const boost::optional<uint64_t>& null_values = {}) : state(s), num_null_values(null_values) {}

        QString message() const
        {
            bool has_null = num_null_values.has_value();

            switch(state)
            {
                case State::NoData:
                    return "No Data Loaded";
                case State::Loading:
                    return "Loading...";
                case State::Loaded:
                    return "Loaded" + (has_null ? " with " + QString::number(num_null_values.value()) + " NULL value(s)" : "");
                case State::ReloadRequired:
                    return "Reload Required";
            }
            return "";
        }

        QColor color() const 
        {
            switch(state)
            {
                case State::NoData:
                    return Qt::black;
                case State::Loading:
                    return Qt::black;
                case State::Loaded:
                    return Qt::black;
                case State::ReloadRequired:
                    return Qt::red;
            }
            return Qt::black;
        }

        bool isLoading() const
        {
            return state == State::Loading;
        }

        State                     state = State::NoData;
        boost::optional<uint64_t> num_null_values;
    };
}
