/************************************************************************
 * Copyright(c) 2013, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

// started 2013-05-05

// put the stuff into a database table at some point in time.

// sources:
// http://www.currency-iso.org/en/home/tables/table-a1.html

// get common currency lists from IB or IQFeed.

#pragma once

#include <string>
#include <map>

namespace ou { // One Unified

class CurrencyCode {
public:
  CurrencyCode(void);
  ~CurrencyCode(void);
protected:
private:
  typedef std::map<std::string,std::string> m_mapCurrencies_t;
  m_mapCurrencies_t m_mapCurrencies;  // currency code, currency name
};

} // namespace ou
