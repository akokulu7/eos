/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2017 Danny van Dyk
 *
 * This file is part of the EOS project. EOS is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * EOS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <eos/statistics/test-statistic.hh>
#include <eos/statistics/test-statistic-impl.hh>

namespace eos
{
    TestStatistic::~TestStatistic()
    {
    }

    namespace test_statistics
    {
        Empty::Empty() = default;

        Empty::~Empty() = default;

        void
        Empty::output(std::ostream & stream) const
        {
            stream << "No test statistic available" << std::endl;
        }

        ChiSquare::ChiSquare(const double & value) :
            value(value)
        {
        }

        ChiSquare::~ChiSquare() = default;

        void
        ChiSquare::output(std::ostream & stream) const
        {
            stream << "chi^2 = " << value << std::endl;
        }
    }
}
